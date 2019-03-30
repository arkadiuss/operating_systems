#define _XOPEN_SOURCE_EXTENDED 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <wait.h>
#include "monitor.h"
#include "child_manager.h"
#include <sys/resource.h>

int cpu_limit = -1, mem_limit = -1;
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

child* observe_file(char *file_and_time){
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
    int pid;
    if(mem_limit == -1 && cpu_limit == -1)
        pid = observe(file_name, atoi(interval), observing_mode);
    else
        pid = observe_restricted(file_name, atoi(interval), observing_mode, cpu_limit, mem_limit);
    return create_child(file_name, pid);
}

child** get_and_observe_files(const char *files_list) {
    FILE *file;
    if((file = fopen(files_list, "r")) == NULL) {
        fprintf(stderr, "Unable to open files list");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    child** children = create_children_table((size_t) file_size);
    int i = 0;
    char *line;
    size_t size = 0;
    while(getline(&line, &size, file) != -1){
        children[i++] = observe_file(line);
    }
    free(line);
    fclose(file);
    return children;
}

double to_milis(struct timeval tm) {
    return ((double) tm.tv_usec)/1000.0 + tm.tv_sec*1000.0;
}

int main(int argc, char **argv) {
    if(argc < 3 || argc == 4) {
        fprintf(stderr, "Wrong arguments count");
        return 1;
    }
    observing_mode = get_mode(argv[2]);
    if(argc > 4) {
        cpu_limit = get_integer(argv[3]);
        mem_limit = get_integer(argv[4]);
    }

    child** children = get_and_observe_files(argv[1]);
    char command[10];
    while(1) {
        scanf("%s", command);
        if(strcmp(command, "LIST") == 0) {
            list_processes(children);
        } else if(strcmp(command, "STOP") == 0) {
            scanf("%s", command);
            if(strcmp(command, "ALL") == 0) {
                stop_all_processes(children);
            } else if(is_integer(command)){
                int pid = atoi(command);
                stop_process(get_child_by_pid(children, pid));
            } else {
                fprintf(stderr, "Should be ALL or pid\n");
            }
        } else if(strcmp(command, "START") == 0) {
            scanf("%s", command);
            if(strcmp(command, "ALL") == 0) {
                start_all_processes(children);
            } else if(is_integer(command)){
                int pid = atoi(command);
                start_process(get_child_by_pid(children, pid));
            } else {
                fprintf(stderr, "Should be ALL or pid\n");
            }
        } else if(strcmp(command, "END") == 0) {
            end_processes(children);
            clear_children(children);
            return 0;
        } else {
            fprintf(stderr, "Unknown command");
        }
    }
    return 0;
}
