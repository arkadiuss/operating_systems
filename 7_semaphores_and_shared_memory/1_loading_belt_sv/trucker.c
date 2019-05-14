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
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>


int X, K, M;
int shmid, semid;
shared_memory *belt;

char buff[100];

void logT(const char* log) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
	printf("%ld: %s", time_in_micros, log);
}

void open_ipc() {
    //SHARED MEMORY
    key_t shmkey;
    if((shmkey = get_shm_key()) == -1) {
        show_error_and_exit("Unable to get shared memory key", 1);
    }

    if((shmid = shmget(shmkey, sizeof(shared_memory), 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        shmctl(shmid, IPC_RMID, 0);
        show_error_and_exit("Unable to get shared memory", 1);
    }

    if((belt = shmat(shmid, 0, 0)) == (void*) -1) {
        show_error_and_exit("Unable to access shared memory", 1);
    }

    //SEMAPHORES
    key_t semkey = get_sem_key();
    if((semid = semget(semkey, SEM_NUM, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        semctl(semid, IPC_RMID, 0);
        show_error_and_exit("Unable to get semaphores", 1);
    }
}

void close_ipc() {
    if(semctl(semid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Unable to remove semaphores\n");
    }

    if(shmdt(belt) == -1)  {
        fprintf(stderr, "Unable to detach memory\n");
    }

    if(shmctl(shmid, IPC_RMID, 0) == -1) {
        show_error_and_exit("Unable to remove shared memory", 1);
    }
}

void int_handler(int signum) {
    close_ipc();
    exit(0);
}

void init_sems_and_shm(int K, int M) {
    belt -> n = 0;
    union semun
    {
        int val;
        struct semid_ds *buf;
        short array [1];
    } sem_attr;
    sem_attr.val = K;
    if(semctl(semid, 0, SETVAL, sem_attr) == -1){
        fprintf(stderr, "Unable to send K value\n");
    }
    sem_attr.val = M;
    if(semctl(semid, 1, SETVAL, sem_attr) == -1){
        fprintf(stderr, "Unable to send M value\n");
    }
    sem_attr.val = 1;
    if(semctl(semid, 2, SETVAL, sem_attr) == -1){
        fprintf(stderr, "Unable to send binary value\n");
    }
}

void load_trucks() {
    struct sembuf shmops[1];
    shmops[0].sem_num = SEM_BELT;
    shmops[0].sem_op = -1;
    shmops[0].sem_flg = 0;
	shmops[1].sem_num = 3;
    shmops[1].sem_op = -1;
    shmops[1].sem_flg = 0;
    
    struct sembuf semops [2];
    semops[0].sem_num = SEM_BELT_SIZE;
    semops[0].sem_op = 1;
    semops[0].sem_flg = 0;
    semops[1].sem_num = SEM_BELT_WEIGHT;
    semops[1].sem_flg = 0;
    int truck_load = 0;
    while(1) {
        shmops[0].sem_op = -1;
    	shmops[1].sem_op = -1;
    	sprintf(buff, "Checking belt...\n");
    	logT(buff);
        if(semop(semid, shmops, 2) == -1) {
            fprintf(stderr, "Unable to access shared memory\n");
        }
        int i = 0;
        while(i < belt->n) {
            semops[1].sem_op = belt->boxes[i].w;
            if (semop(semid, semops, 2) == -1) {
                fprintf(stderr, "Unable to get pack\n");
            }
            box b = belt->boxes[i];
            belt->w -= b.w;
            sprintf(buff, "Loading to truck pack with weight %d from %d\n", b.w, b.id);
            logT(buff);
            i++;
            truck_load++;
            sprintf(buff, "Truck loaded in %d/%d\n", truck_load, X);
            logT(buff);
            if(truck_load == X) {
                sprintf(buff, "Truck fully loaded. Swapping...\n");
            	logT(buff);
                truck_load = 0;
            }
        }
        belt->n = 0;
        shmops[0].sem_op = 1;
        if(semop(semid, shmops, 1) == -1) {
            fprintf(stderr, "Unable to release shared memory\n");
        }
    }
}

int main(int argc, char **argv) {
    validate_argc(argc, 3);
    X = as_integer(argv[1]);
    K = as_integer(argv[2]);
    M = as_integer(argv[3]);

    //SIGNALS
    struct sigaction intact;
    intact.sa_handler = int_handler;
    sigemptyset(&intact.sa_mask);
    sigaction(SIGINT, &intact, NULL);

    open_ipc();
    init_sems_and_shm(K, M);
    load_trucks();
    close_ipc();
    return 0;
}
