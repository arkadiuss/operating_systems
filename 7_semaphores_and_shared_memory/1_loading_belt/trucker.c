#include <stdio.h>
#include <sys-ops-commons.h>
#include "belt_loader.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <signal.h>

int X, K, M;
int shmid, semid;
void* packs;

void open_ipc() {
    //SHARED MEMORY
    key_t shmkey;
    if((shmkey = get_shm_key()) == -1) {
        show_error_and_exit("Unable to get shared memory key", 1);
    }

    if((shmid = shmget(shmkey, MEM_SIZE, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        shmctl(shmid, IPC_RMID, 0);
        show_error_and_exit("Unable to get shared memory", 1);
    }

    if((packs = shmat(shmid, 0, 0)) == (void*) -1) {
        show_error_and_exit("Unable to access shared memory", 1);
    }

    //SEMAPHORES
    key_t semkey = get_sem_key();
    if((semid = semget(semkey, 2, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
        semctl(semid, IPC_RMID, 0);
        show_error_and_exit("Unable to get semaphores", 1);
    }
}

void close_ipc() {
    if(semctl(semid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Unable to remove semaphores\n");
    }

    if(shmdt(packs) == -1)  {
        fprintf(stderr, "Unable to detach memory\n");
    }

    if(shmctl(shmid, IPC_RMID, 0) == -1) {
        show_error_and_exit("Unable to remove shared memory", 1);
    }
}

void int_handler(int signum) {
    shmctl(shmid, IPC_RMID, 0);
    close_ipc();
}

void load_trucks() {
    int c = 5;
    while(c--) {
        printf("Loop\n");
        sleep(2);
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
    load_trucks();
    close_ipc();
    return 0;
}