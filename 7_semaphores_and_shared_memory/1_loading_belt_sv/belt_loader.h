//
// Created by arkadius on 12.05.19.
//

#ifndef LOADINGBELT_BELT_H
#define LOADINGBELT_BELT_H

#include <sys/types.h>
#include <sys/types.h>
#include <sys/ipc.h>

const int PROJECT_ID = 13554;
const char *SHM_PATH = "/tmp/shm_key";
const char *SEM_PATH = "/tmp/sem_key";
#define MAX_BELT_SIZE 1024
#define SEM_NUM 3
#define SEM_BELT_SIZE 0
#define SEM_BELT_WEIGHT 1
#define SEM_BELT 2


key_t get_shm_key(){
    return ftok(SHM_PATH, PROJECT_ID);
}

key_t get_sem_key(){
    return ftok(SEM_PATH, PROJECT_ID);
}

typedef struct box {
    int id;
    int w;
} box;

typedef struct shared_memory {
    box boxes[MAX_BELT_SIZE];
    int n, w;
} shared_memory;

#endif //LOADINGBELT_BELT_H
