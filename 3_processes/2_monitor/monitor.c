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
    if(stat(path, &stats) == -1){
        return -1;
    }
    return stats.st_mtime;
}

char* last_index_of(char *name, char c) {
    char* ptr = strrchr(name, c);
    return ptr == NULL ? name : ptr +
    1;
}

long get_file_size(FILE *file) {
    long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

char* read_file(const char *file_name){
    FILE *file;
    if((file = fopen(file_name, "r")) != NULL) {
        int c;
        long size = get_file_size(file);
        char* buffer = malloc((size_t) size + 1);
        long i = 0;
        while((c = getc(file)) != EOF) {
            buffer[i++] = (char) c;
        }
        buffer[i] = '\0';
        fclose(file);
        return buffer;
    } else {
        fprintf(stderr, "Unable to open file to read\n");
        return NULL;
    }
}

void write_file(const char *file_name, char *buffer, time_t mod) {
    FILE *file;
    char new_path[1024];
    char *date = get_date(mod);
    sprintf(new_path, "%s/%s_%s", BACKUP_FOLDER, last_index_of(file_name, '/'), date);
    free(date);
    if((file = fopen(new_path, "w")) != NULL) {
        fwrite(buffer, sizeof(char), (size_t) strlen(buffer), file);
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open file to write \n");
    }
}

void observe_file_archive(const char *file_name, int interval, int lifespan){
    int remainingTime = lifespan;
    time_t last_mod = -1;
    int n = 0;
    char *content = NULL;
    while(remainingTime > 0) {
        time_t mod = get_modification_time(file_name);
        if(mod != last_mod) {
            fflush(stdout);
            if(content != NULL) {
                write_file(file_name, content, last_mod);
                n++;
                free(content);
            }
            content = read_file(file_name);
            last_mod = mod;
            printf("Read file %s\n", content);
        }
        int sleepTime = remainingTime < interval ? remainingTime : interval;
        sleep(sleepTime);
        remainingTime -= sleepTime;
    }
    if(content != NULL)
        free(content);
    pid_t pid = getpid();
    printf("Process %d created %d copies of file %s and is finished\n", pid, n, file_name);
    exit(0);
}

void make_copy_exec(const char *file_path, time_t last_mod) {
    int pid;
    if((pid = fork()) == 0) {
        char new_path[1024];
        char *date = get_date(last_mod);
        sprintf(new_path, "%s/%s_%s", BACKUP_FOLDER, last_index_of(file_path, '/'), date);
        free(date);
        if(execlp("cp", "cp", file_path, new_path, NULL) == -1) {
            fprintf(stderr, "Copy with exec errored: %d", errno);
            exit(1);
        }
    }
}

void observe_file_exec(const char *file_name, int interval, int lifespan){
    int remainingTime = lifespan;
    time_t last_mod = -1;
    int n = 0;
    while(remainingTime > 0) {
        time_t mod = get_modification_time(file_name);
        if(mod != last_mod) {
            printf("Making copy of %s with exec\n", file_name);
            fflush(stdout);
            make_copy_exec(file_name, mod);
            n++;
            last_mod = mod;
        }
        int sleepTime = remainingTime < interval ? remainingTime : interval;
        sleep(sleepTime);
        remainingTime -= sleepTime;
    }
    pid_t pid = getpid();
    printf("Process %d created %d copies of file %s and is finished\n", pid, n, file_name);
    exit(0);
}

void observe(const char *file_name, int interval, int lifespan, Mode mode) {
    printf("I will observe %s for %ds every %ds\n", file_name, lifespan, interval);
    int pid;
    if((pid = fork()) == 0) {
        switch (mode) {
            case ARCHIVE:
                observe_file_archive(file_name, interval, lifespan);
                break;
            case COPY:
                observe_file_exec(file_name, interval, lifespan);
                break;
        }
    }
}
