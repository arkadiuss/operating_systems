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
shared_memory *belt;

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
    if((semid = semget(semkey, 2, 0666 | IPC_CREAT | IPC_EXCL)) < 0) {
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
    shmctl(shmid, IPC_RMID, 0);
    close_ipc();
}

void init_sems_and_shm(K, M) {
    belt -> first = 0;
    belt -> last = 0;
    union semun
    {
        int val;
        struct semid_ds *buf;
        ushort array [1];
    } sem_attr;
    sem_attr.val = K;
    if(semctl(semid, 0, SETVAL, sem_attr) == -1){
        fprintf(stderr, "Unable to send K value");
    }
    sem_attr.val = M;
    if(semctl(semid, 1, SETVAL, sem_attr) == -1){
        fprintf(stderr, "Unable to send M value");
    }
}

void load_trucks() {
    struct sembuf semops [2];
    semops[0].sem_num = 0;
    semops[0].sem_op = 1;
    semops[0].sem_flg = 0;
    semops[1].sem_num = 1;
    semops[1].sem_flg = 0;
    int truck_load = 0;
    while(1) {
        printf("Checking belt...\n");
        while(belt->first < belt->last) {
            semops[1].sem_op = belt->boxes[belt->first].w;
            if (semop(semid, semops, 2) == -1) {
                fprintf(stderr, "Unable to get pack\n");
            }
            box b = belt->boxes[belt->first];
            printf("Loading to truck pack %d with weight %d from %d\n", belt->first, b.w, b.id);
            truck_load++;
            belt->first++;
            printf("Truck loaded in %d/%d\n", truck_load, X);
            if(truck_load == X) {
                printf("Truck fully loaded. Swapping...\n");
                truck_load = 0;
            }
        }
        sleep(1);
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