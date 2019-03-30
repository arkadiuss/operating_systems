//
// Created by arkadius on 20.03.19.
//
#define _XOPEN_SOURCE_EXTENDED 1

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <wait.h>
#include <errno.h>
#include <string.h>
#include <sys/resource.h>
#include "monitor.h"

const char* BACKUP_FOLDER = "./backup";

typedef struct {
    char *content;
    size_t size;
    time_t mod;
} file_content ;

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

const char* last_index_of(const char *name, char c) {
    char* ptr = strrchr(name, c);
    return ptr == NULL ? name : ptr + 1;
}

long get_file_size(FILE *file) {
    long size;
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

int read_file(const char *file_name, file_content* file_content){
    FILE *file;
    if((file = fopen(file_name, "r")) != NULL) {
        file_content->size = (size_t) get_file_size(file);
        file_content->content = malloc((size_t) file_content->size + 1);
        if(fread(file_content->content, sizeof(char), file_content->size, file) != file_content->size) {
            fprintf(stderr, "Unable to read file\n");
            free(file_content->content);
            return -1;
        }
        file_content->content[file_content->size] = '\0';
        fclose(file);
        return 0;
    } else {
        fprintf(stderr, "Unable to open file to read\n");
        return -1;
    }
}

int write_file(const char *file_name, file_content* file_content) {
    FILE *file;
    char new_path[1024];
    char *date = get_date(file_content->mod);
    sprintf(new_path, "%s/%s_%s", BACKUP_FOLDER, last_index_of(file_name, '/'), date);
    free(date);
    if((file = fopen(new_path, "w")) != NULL) {
        fwrite(file_content->content, sizeof(char), file_content->size, file);
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open file to write \n");
        return -1;
    }
    return 0;
}

void observe_file_archive(const char *file_name, int interval){
    time_t last_mod = -1;
    int n = 0;
    file_content file_content;
    file_content.content = NULL;
    while(1) {
        file_content.mod = get_modification_time(file_name);
        if(file_content.mod != last_mod) {
            printf("Making copy of %s\n", file_name);
            if(file_content.content != NULL) {
                if(write_file(file_name, &file_content) != 0) {
                    exit(1);
                }
                n++;
                free(file_content.content);
            }
            if(read_file(file_name, &file_content) != 0) {
                exit(1);
            }
            last_mod = file_content.mod;
        }
        sleep(interval);
    }
    if(file_content.content != NULL)
        free(file_content.content);
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

void observe_file_exec(const char *file_name, int interval){
    time_t last_mod = -1;
    int n = 0;
    while(1) {
        time_t mod = get_modification_time(file_name);
        if(mod != last_mod) {
            printf("Making copy of %s with exec\n", file_name);
            fflush(stdout);
            make_copy_exec(file_name, mod);
            n++;
            last_mod = mod;
        }
        sleep(interval);
    }
    pid_t pid = getpid();
    printf("Process %d created %d copies of file %s and is finished\n", pid, n, file_name);
    exit(0);
}

int observe(const char *file_name, int interval, Mode mode) {
    printf("I will observe %s every %ds\n", file_name, interval);
    int pid;
    if((pid = fork()) == 0) {
        switch (mode) {
            case ARCHIVE:
                observe_file_archive(file_name, interval);
                break;
            case COPY:
                observe_file_exec(file_name, interval);
                break;
        }
    }
    return pid;
}

int observe_restricted(const char *file_name, int interval, Mode mode, int cpu_limit, int memory_limit) {
    printf("I will observe %s every %ds with restrictions %d cpu %d memory\n",
            file_name, interval, cpu_limit, memory_limit);
    int pid;
    if((pid = fork()) == 0) {
        struct rlimit cpu_rlimit, memory_rlimit;
        cpu_rlimit.rlim_cur = (rlim_t) cpu_limit;
        memory_rlimit.rlim_cur = (rlim_t) ((long long) memory_limit) * 1000000LL;
        cpu_rlimit.rlim_max = (rlim_t) cpu_limit;
        memory_rlimit.rlim_max = (rlim_t) ((long long) memory_limit) * 1000000LL;
        setrlimit(RLIMIT_AS, &memory_rlimit);
        setrlimit(RLIMIT_CPU, &cpu_rlimit);
        switch (mode) {
            case ARCHIVE:
                observe_file_archive(file_name, interval);
                break;
            case COPY:
                observe_file_exec(file_name, interval);
                break;
        }
    }
    return pid;
}
