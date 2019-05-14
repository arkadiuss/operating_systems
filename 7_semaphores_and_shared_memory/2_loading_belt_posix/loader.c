#define _POSIX_SOURCE
#define _XOPEN_SOURCE
#include <stdio.h>
#include <sys-ops-commons.h>
#include "belt_loader.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>


int N, C = -1;
int shmfd;
sem_t *sem_belt, *sem_belt_count, *sem_belt_check;
shared_memory *belt;

char buff[100];

void logT(const char* log) {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
	printf("%ld: %s", time_in_micros, log);
}

void read_args(int argc, char **argv) {
    validate_argc(argc, 1);
    N = as_integer(argv[1]);
    if(argc > 2) {
        C = as_integer(argv[2]);
    }
}

void open_posix() {
    //SHARED MEMORY
    if((shmfd = shm_open(SHM_NAME, O_RDWR, 0666)) < 0) {
        show_error_and_exit("Unable to get shared memory", 1);
    }

    if((belt = mmap(NULL, sizeof(shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED) {
        show_error_and_exit("Unable to access shared memory", 1);
    }

    //SEMAPHORES
    if((sem_belt = sem_open(SEM_BELT_NAME, O_RDWR, 0666)) < 0) {
        show_error_and_exit("Unable to get semaphore belt", 1);
    }

    if((sem_belt_count = sem_open(SEM_BELT_COUNT_NAME, O_RDWR, 0666)) < 0) {
        show_error_and_exit("Unable to get semaphore count", 1);
    }
    
    if((sem_belt_check = sem_open(SEM_BELT_CHECK_NAME, O_RDWR, 0666)) < 0) {
        show_error_and_exit("Unable to get semaphore count", 1);
    }
}
void load_packs() {
    int c = C;
    while(c--){
        sprintf(buff, "Waiting for pack to load by %d\n", getpid());
        logT(buff);
        if(sem_wait(sem_belt_count) == -1) {
            fprintf(stderr, "Unable to perform action on semaphores\n");
        }
        if(sem_wait(sem_belt) == -1) {
            fprintf(stderr, "Unable to access shared memory\n");
        }
        if(belt->w + N <= belt->maxW) {
            box b;
            b.w = N;
            b.id = getpid();
            belt->boxes[belt->n++] = b;
            belt->w += N;
            sprintf(buff, "Pack %d with weight %d loaded by %d\n", belt->n - 1, N, getpid());
            logT(buff);
            sprintf(buff, "Belt state - load: %d, weight: %d\n", belt->n, belt->w);
            logT(buff);
            if(sem_post(sem_belt_check) == -1) {
                fprintf(stderr, "Unable to release action on semaphore check\n");
            }
        } else {
            if(sem_post(sem_belt_count) == -1) {
                fprintf(stderr, "Unable to release action on semaphore count\n");
            }
            sleep(1);
        }
        if(sem_post(sem_belt) == -1) {
            fprintf(stderr, "Unable to release shared memory\n");
        }
    }
}

int main(int argc, char **argv) {
    read_args(argc, argv);

    open_posix();
    load_packs();
    return 0;
}
