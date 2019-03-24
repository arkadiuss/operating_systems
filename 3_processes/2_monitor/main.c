#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include "monitor.h"

int observing_time, cpu_limit = -1, mem_limit = -1;
Mode observing_mode;

int is_integer(char* text){
    int i = 0;
    char c;
    while((c = text[i++]) != '\0'){
        if(c < 48 || c > 57) {
            fprintf(stderr, "%s in not an integer\n", text);
            return 0;
        }
    }
    return 1;
}

int get_integer(char *i) {
    if(!is_integer(i)){
        exit(1);
    }
    return atoi(i);
}

Mode get_mode(char *mode){
    if(strcmp(mode, "archive") == 0) {
        return ARCHIVE;
    } else if(strcmp(mode, "copy") == 0) {
        return COPY;
    } else {
        fprintf(stderr, "Unknown mode");
        exit(1);
    }
}

void remove_new_line(char *str){
    int i = 0;
    while(str[i] != '\0') {
        if(str[i] == '\n')
            str[i] = '\0';
        i++;
    }
}

void observe_file(char *file_and_time){
    char *file_name, *interval;
    const char *delimiter = " ";
    file_name = strtok(file_and_time, delimiter);
    interval = strtok(NULL, delimiter);
    if(file_name == NULL || interval == NULL) {
        fprintf(stderr, "File should consist file name and time in every line");
        exit(1);
    }
    remove_new_line(interval);
    if(!is_integer(interval)){
        exit(1);
    }
    if(mem_limit == -1 && cpu_limit == -1)
        observe(file_name, atoi(interval), observing_time, observing_mode);
    else
        observe_restricted(file_name, atoi(interval), observing_time, observing_mode, cpu_limit, mem_limit);
}

void get_and_observe_files(const char *files_list) {
    FILE *file;
    if((file = fopen(files_list, "r")) == NULL) {
        fprintf(stderr, "Unable to open files list");
        exit(1);
    }
    char *line;
    size_t size = 0;
    while(getline(&line, &size, file) != -1){
        observe_file(line);
    }
    free(line);
    fclose(file);
}

int main(int argc, char **argv) {
    if(argc < 4 || argc == 5) {
        fprintf(stderr, "Wrong arguments count");
        return 1;
    }
    observing_mode = get_mode(argv[3]);
    observing_time = get_integer(argv[2]);
    if(argc > 4) {
        cpu_limit = get_integer(argv[4]);
        mem_limit = get_integer(argv[5]);
    }
    get_and_observe_files(argv[1]);
    int status;
    wait(&status);

    return 0;
}