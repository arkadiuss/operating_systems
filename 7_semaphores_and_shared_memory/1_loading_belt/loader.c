#include <stdio.h>
#include <sys-ops-commons.h>
#include "belt_loader.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>

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
    if((semid = semget(semkey, 2, 0666)) < 0) {
        show_error_and_exit("Unable to get semaphores", 1);
    }
}

void close_ipc() {
    if(shmdt(belt) == -1)  {
        fprintf(stderr, "Unable to detach memory \n");
    }
}

void load_packs() {
    int c = C;
    struct sembuf semops [2];
    semops[0].sem_num = 0;
    semops[0].sem_op = -1;
    semops[0].sem_flg = 0;
    semops[1].sem_num = 1;
    semops[1].sem_op = -N;
    semops[1].sem_flg = 0;
    while(c--){
        printf("Waiting for pack to load by %d\n", getpid());
        if(semop(semid, semops, 2) == -1) {
            fprintf(stderr, "Unable to perform action on semaphores\n");
        }
        box b;
        b.w = N;
        b.id = getpid();
        belt->boxes[belt->last++] = b;
        printf("Pack %d with weight %d loaded by %d\n", belt->last - 1, N, getpid());
    }
}

int main(int argc, char **argv) {
    read_args(argc, argv);

    open_ipc();
    load_packs();
    close_ipc();
    return 0;
}