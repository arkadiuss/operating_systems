//
// Created by arkadius on 15.03.19.
//
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include "files_operations.h"


void fill_with_random(char *record, int record_size){
    for(int i = 0; i < record_size; i++){
        record[i] = (char) rand()%26+65;
    }
}

int generate(char *file_name, int records_count, int record_size) {
    time_t t;
    srand(time(&t));
    int fd = open(file_name, O_CREAT | O_WRONLY);
    if(fd == -1) {
        fprintf(stderr, "Error while opening file");
        return -1;
    }
    char* record = malloc(sizeof(char)*record_size);
    while(records_count--){
        fill_with_random(record, record_size);
        printf("%s", record);
        write(fd, record, record_size);
    }
    close(fd);
    free(record);
    return 0;
}

int sys_sort(char *file_name, int records_count, int record_size) {
    return 0;
}

int lib_sort(char *file_name, int records_count, int record_size) {
    return 0;
}

int sys_copy(char *file_name_src, char *file_name_dest, int records_count, int record_size) {
    return 0;
}

int lib_copy(char *file_name_src, char *file_name_dest, int records_count, int record_size) {
    return 0;
}
