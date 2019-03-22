//
// Created by arkadius on 20.03.19.
//

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
#include "monitor.h"

const char* BACKUP_FOLDER = "./backup";

char *get_date(time_t time){
    struct tm * timeinfo;
    timeinfo = localtime ( &time );
    char *buff = malloc(sizeof(char) * 20);
    strftime(buff, 20, "%Y-%m-%d_%H-%M-%S", timeinfo);
    return buff;
}

time_t get_modification_time(const char* path) {
    struct stat stats;
    stat(path, &stats);
    return stats.st_mtime;
}

char* lastIndexOf(char *name, char c) {
    char* ptr = strrchr(name, c);
    return ptr == NULL ? name : ptr +
    1;
}

void makeCopyExec(const char *file_path, time_t last_mod) {
    int pid;
    if((pid = fork()) == 0) {
        char new_path[1024];
        char *date = get_date(last_mod);
        sprintf(new_path, "%s/%s_%s", BACKUP_FOLDER, lastIndexOf(file_path, '/'), date);
        free(date);
        if(execlp("cp", "cp", file_path, new_path, NULL) == -1) {
            fprintf(stderr, "Copy with exec errored: %d", errno);
            exit(1);
        }
    }
}

void observeFileArchive(const char *file_name, int interval, int lifespan){
    int remainingTime = lifespan;
    while(remainingTime > 0) {


        int sleepTime = remainingTime < interval ? remainingTime : interval;
        sleep(sleepTime);
        remainingTime -= sleepTime;
    }
}

void observeFileExec(const char *file_name, int interval, int lifespan){
    int remainingTime = lifespan;
    time_t last_mod = -1;
    int n = 0;
    while(remainingTime > 0) {
        time_t mod = get_modification_time(file_name);
        if(mod != last_mod) {
            printf("Making copy of %s with exec\n", file_name);
            fflush(stdout);
            makeCopyExec(file_name, mod);
            n++;
            last_mod = mod;
        }
        int sleepTime = remainingTime < interval ? remainingTime : interval;
        sleep(sleepTime);
        remainingTime -= sleepTime;
    }
    pid_t pid = getpid();
    printf("Process %d created %d copies of file %s and is finished\n", pid, n, file_name);
}

void observe(const char *file_name, int interval, int lifespan, Mode mode) {
    printf("I will observe %s for %ds every %ds\n", file_name, lifespan, interval);
    int pid;
    if((pid = fork()) == 0) {
        switch (mode) {
            case ARCHIVE:
                observeFileArchive(file_name, interval, lifespan);
                break;
            case COPY:
                observeFileExec(file_name, interval, lifespan);
                break;
        }
    } else {
        printf("I %d created a child!", getpid());
    }
}
