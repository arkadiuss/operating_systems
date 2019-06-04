#include "sys-ops-commons.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void validate_argc(int argc, int required) {
    if(argc < required + 1){
        fprintf(stderr, "Wrong arguments count\n");
        exit(1);
    }
}

long get_file_size(const char *file_name) {
    int fd;
    if((fd = open(file_name, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open file for size \n");
        return -1;
    }
    struct stat stats;
    fstat(fd, &stats);
    long size = stats.st_size;
    close(fd);
    return size;
}

long read_whole_file(const char *file_name, char *buffer) {
    long size = get_file_size(file_name);
    if(size == -1) {
        return size;
    }
    FILE *file;
    if((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "Unable to open file \n");
        return -1;
    }
    long read_size;
    if((read_size = fread(buffer, sizeof(char), (size_t) size, file)) != size) {
        fprintf(stderr, "Unable to read file exp: %ld, act: %ld\n", size, read_size);
        return -1;
    }
    fclose(file);
    return read_size;
}

int is_integer(char* str){
    int i = 0;
    char c;
    while((c = str[i++]) != '\0'){
        if(c < 48 || c > 57) {
            fprintf(stderr, "%s in not an integer\n", str);
            return 0;
        }
    }
    return 1;
}

int is_integer_list(char* str){
    int i = 0;
    char c;
    while((c = str[i++]) != '\0'){
        if((c < 48 || c > 57) && c != 32) {
            fprintf(stderr, "%s in not an integer list. sign: \"%d\", pos: %d\n", str, c, i-1);
            return 0;
        }
    }
    return 1;
}

int as_integer(char *str) {
    if(!is_integer(str)) {
        exit(1);
    }
    return atoi(str);
}

void show_error_and_exit(const char *err, int exit_code) {
    fprintf(stderr, "Error: %s Strerr: %s\n", err, strerror(errno));
    exit(exit_code);
}

int split_to_arr(char **args, char *msg) {
    char *str;
    str = strtok(msg, " ");
    int c = 0;
    while(str != NULL) {
        (*args++) = strdup(str);
        str = strtok(NULL, " ");
        c++;
    }
    return c;
}

int max(int a, int b){
    return a > b ? a : b;
}

int min(int a, int b){
    return a < b ? a : b;
}