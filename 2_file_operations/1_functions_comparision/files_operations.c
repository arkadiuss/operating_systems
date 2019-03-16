//
// Created by arkadius on 15.03.19.
//
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "files_operations.h"


void fill_with_random(char *record, int record_size){
    for(int i = 0; i < record_size; i++){
        record[i] = (char) (rand()%25+65);
    }
}

int generate(char *file_name, int records_count, int record_size) {
    time_t t;
    srand(time(&t));
    int fd = open(file_name, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
    if(fd == -1) {
        fprintf(stderr, "Error while opening file %d", errno);
        return -1;
    }
    char* record = malloc(sizeof(char)*record_size);
    while(records_count--){
        fill_with_random(record, record_size);
        write(fd, record, record_size);
    }
    close(fd);
    free(record);
    return 0;
}

int sys_sort(char *file_name, int records_count, int record_size) {
    int fd = open(file_name, O_RDWR);
    if(fd == -1) {
        fprintf(stderr, "Unable to open requested file");
        return -1;
    }
    char *nxt_record = malloc(sizeof(char) * record_size);
    char *min_record = malloc(sizeof(char) * record_size);
    for(int cur = 0; cur < records_count; cur++){
        lseek(fd, cur*record_size, SEEK_SET);
        read(fd, min_record, record_size);
        int min_off = cur;
        for(int nxt = cur + 1; nxt < records_count; nxt++){
            read(fd, nxt_record, 1);
            if(nxt_record[0] < min_record[0]){
                min_record[0] = nxt_record[0];
                read(fd, min_record + 1, record_size - 1);
                min_off = nxt;
            } else {
                lseek(fd, record_size - 1, SEEK_CUR);
            }
        }
        lseek(fd, cur*record_size, SEEK_SET);
        read(fd, nxt_record, record_size);
        lseek(fd, (min_off - cur - 1)*record_size, SEEK_CUR);
        write(fd, nxt_record, record_size);
        lseek(fd, cur*record_size, SEEK_SET);
        write(fd, min_record, record_size);
    }
    free(nxt_record);
    free(min_record);
    return 0;
}

int lib_sort(char *file_name, int records_count, int record_size) {
    return 0;
}

int sys_copy(char *file_name_src, char *file_name_dest, int records_count, int record_size) {
    int fd_src = open(file_name_src, O_RDONLY);
    int fd_dest = open(file_name_dest, O_RDWR|O_CREAT, S_IRWXU|S_IRGRP|S_IROTH);
    if(fd_src == -1 || fd_dest == -1) {
        fprintf(stderr, "Unable to open src or dest file");
        return -1;
    }
    char* record = malloc(sizeof(char)*record_size);
    ssize_t readed_size;
    while(records_count-- &&
        (readed_size = read(fd_src, record, record_size)) != 0) {
        write(fd_dest, record, readed_size);
    }
    free(record);
    return 0;
}

int lib_copy(char *file_name_src, char *file_name_dest, int records_count, int record_size) {
    FILE *src = fopen(file_name_src, "r");
    FILE *dest = fopen(file_name_dest, "w");
    if(src == NULL || dest == NULL) {
        fprintf(stderr, "Unable to open src or dest file");
        return -1;
    }
    char* record = malloc(sizeof(char)*record_size);
    ssize_t readed_size;
    while(records_count-- &&
        (readed_size = fread(record, sizeof(char), record_size, src)) != 0) {
        fwrite(record, sizeof(char), readed_size, dest);
    }
    free(record);
    return 0;
}
