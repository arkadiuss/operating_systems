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
const int MEM_SIZE = 1024;

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

#endif //LOADINGBELT_BELT_H
