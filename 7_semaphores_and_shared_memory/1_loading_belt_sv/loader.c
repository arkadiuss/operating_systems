#define _POSIX_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <sys-ops-commons.h>
#include "belt_loader.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/time.h>

int N, C = -1;
int semid, shmid;
shared_memory *belt;

void read_args(int argc, char **argv) {
    validate_argc(argc, 1);
    N = as_integer(argv[1]);
    if(argc > 2) {
        C = as_integer(argv[2]);
    }
}

char buff[100];

void logT(const char* log) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
	printf("%ld: %s", time_in_micros, log);
}

void open_ipc() {
    //SHARED MEMORY
    key_t shmkey = get_shm_key();
    if((shmid = shmget(shmkey, 0, 0666)) < 0) {
        show_error_and_exit("Unable to get shared memory", 1);
    }

    if((belt = shmat(shmid, 0, 0)) == (void*) -1) {
        show_error_and_exit("Unable to access shared memory", 1);
    }

    //SEMAPHORES
    key_t semkey = get_sem_key();
    if((semid = semget(semkey, SEM_NUM, 0666)) < 0) {
        show_error_and_exit("Unable to get semaphores", 1);
    }
}
void load_packs() {
    int c = C;
    struct sembuf semops[2];
    semops[0].sem_num = SEM_BELT_SIZE;
    semops[0].sem_op = -1;
    semops[0].sem_flg = 0;
    semops[1].sem_num = SEM_BELT_WEIGHT;
    semops[1].sem_op = -N;
    semops[1].sem_flg = 0;
    struct sembuf shmsem[1];
    shmsem[0].sem_num = SEM_BELT;
    shmsem[0].sem_op = -1;
    shmsem[0].sem_flg = 0;
	shmsem[1].sem_num = 3;
    shmsem[1].sem_op = 1;
    shmsem[1].sem_flg = 0;
    while(c--){
        sprintf(buff, "Waiting for pack to load by %d\n", getpid());
        logT(buff);
        if(semop(semid, semops, 2) == -1) {
            fprintf(stderr, "Unable to perform action on semaphores\n");
        }
        shmsem[0].sem_op = -1;
        if(semop(semid, shmsem, 2) == -1) {
            fprintf(stderr, "Unable to access shared memory\n");
        }
        box b;
        b.w = N;
        b.id = getpid();
        belt->boxes[belt->n++] = b;
        belt->w += N;
        sprintf(buff, "Pack %d with weight %d loaded by %d\n", belt->n - 1, N, getpid());
        logT(buff);
        sprintf(buff, "Belt state - load: %d, weight: %d\n", belt->n, belt->w);
        logT(buff);
        shmsem[0].sem_op = 1;
        if(semop(semid, shmsem, 1) == -1) {
            fprintf(stderr, "Unable to release shared memory\n");
        }
    }
}

int main(int argc, char **argv) {
    read_args(argc, argv);

    open_ipc();
    load_packs();
    return 0;
}
