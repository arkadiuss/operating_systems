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

size_t get_file_size(const char *file_name) {
    int fd;
    if((fd = open(file_name, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open file for size \n");
        return (size_t) -1;
    }
    struct stat stats;
    fstat(fd, &stats);
    size_t size = (size_t) stats.st_size;
    close(fd);
    return size;
}

size_t read_whole_file(const char *file_name, char *buffer) {
    size_t size = get_file_size(file_name);
    if(size == -1) {
        return size;
    }
    FILE *file;
    if((file = fopen(file_name, "r")) == NULL) {
        fprintf(stderr, "Unable to open file \n");
        return (size_t) -1;
    }
    size_t read_size;
    if((read_size = fread(buffer, sizeof(char), size, file)) != size) {
        fprintf(stderr, "Unable to read file\n");
        return (size_t) -1;
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

void show_error_and_exit(const char *err, int exit_code) {
    fprintf(stderr, "Error: %s Strerr: %s", err, strerror(errno));
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