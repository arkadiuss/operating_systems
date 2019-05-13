#include <stdio.h>
#include <sys-ops-commons.h>
#include "belt_loader.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

int X, K, M;
int shmfd;
sem_t *sem_belt, *sem_belt_count;
shared_memory *belt;

void open_posix(int K) {
    //SHARED MEMORY
    if((shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0666)) < 0) {
        show_error_and_exit("Unable to get shared memory", 1);
    }

    if(ftruncate(shmfd, sizeof(shared_memory)) == -1) {
        show_error_and_exit("Unable to truncate shared memory", 1);
    }

    if((belt = mmap(NULL, sizeof(shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
        show_error_and_exit("Unable to access shared memory", 1);
    }

    //SEMAPHORES
    if((sem_belt = sem_open(SEM_BELT_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, 1)) < 0) {
        show_error_and_exit("Unable to get semaphore belt", 1);
    }

    if((sem_belt_count = sem_open(SEM_BELT_COUNT_NAME, O_RDWR | O_CREAT | O_EXCL, 0666, K)) < 0) {
        show_error_and_exit("Unable to get semaphore count", 1);
    }
}

void close_posix() {
    if(sem_unlink(SEM_BELT_NAME) == -1) {
        fprintf(stderr, "Unable to remove semaphore belt\n");
    }

    if(sem_unlink(SEM_BELT_COUNT_NAME) == -1) {
        fprintf(stderr, "Unable to remove semaphore count\n");
    }

    if(shm_unlink(SHM_NAME) == -1) {
        show_error_and_exit("Unable to remove shared memory", 1);
    }
}

void int_handler(int signum) {
    close_posix();
    exit(0);
}

void init_sems_and_shm(int M) {
    belt -> n = 0;
    belt -> maxW = M;
}

void load_trucks() {
    int truck_load = 0;
    while(1) {
        printf("Checking belt...\n");
        if(sem_wait(sem_belt) == -1) {
            fprintf(stderr, "Unable to access shared memory\n");
        }
        int i = 0;
        while(i < belt->n) {
            if (sem_post(sem_belt_count) == -1) {
                fprintf(stderr, "Unable to get pack\n");
            }
            box b = belt->boxes[i];
            belt->w -= b.w;
            printf("Loading to truck pack with weight %d from %d\n", b.w, b.id);
            i++;
            truck_load++;
            printf("Truck loaded in %d/%d\n", truck_load, X);
            if(truck_load == X) {
                printf("Truck fully loaded. Swapping...\n");
                truck_load = 0;
            }
        }
        belt->n = 0;
        if(sem_post(sem_belt) == -1) {
            fprintf(stderr, "Unable to release shared memory\n");
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

    open_posix(K);
    init_sems_and_shm(M);
    load_trucks();
    close_posix();
    return 0;
}