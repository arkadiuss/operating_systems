#include <stdio.h>
#include <sys-ops-commons.h>
#include "belt_loader.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

int N, C = -1;
int semid, shmid;
void *packs;

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

    if((packs = shmat(shmid, 0, 0)) == (void*) -1) {
        show_error_and_exit("Unable to access shared memory", 1);
    }

    //SEMAPHORES
    key_t semkey = get_sem_key();
    if((semid = semget(semkey, 2, 0666)) < 0) {
        show_error_and_exit("Unable to get semaphores", 1);
    }
}

void close_ipc() {
    if(shmdt(packs) == -1)  {
        fprintf(stderr, "Unable to detach memory \n");
    }
}

void load_packs() {
    printf("Loading packs\n");
}

int main(int argc, char **argv) {
    read_args(argc, argv);

    open_ipc();
    load_packs();
    close_ipc();
    return 0;
}