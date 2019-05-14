//
// Created by arkadius on 12.05.19.
//

#ifndef LOADINGBELT_BELT_H
#define LOADINGBELT_BELT_H

const int PROJECT_ID = 13554;
const char *SHM_NAME = "/BELT_SHM4";
const char *SEM_BELT_NAME = "/BELT_SEM4";
const char *SEM_BELT_COUNT_NAME = "/BELT_COUNT_SEM4";
const char *SEM_BELT_CHECK_NAME = "/BELT_CHECK_SEM4";
#define MAX_BELT_SIZE 1024

typedef struct box {
    int id;
    int w;
} box;

typedef struct shared_memory {
    box boxes[MAX_BELT_SIZE];
    int n, w;
    int maxW;
} shared_memory;

#endif //LOADINGBELT_BELT_H
