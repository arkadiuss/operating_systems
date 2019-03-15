#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "files_operations.h"

int generate_file(char **args){
    char *file_name = args[2];
    char *records_count = args[3];
    char *record_size = args[4];
    generate(file_name, atoi(records_count), atoi(record_size));
}

int check_type(char *type) {
    if(strcmp(type, "lib") == 0) {
        return 0;
    } else if(strcmp(type, "sys")){
        return 1;
    } else {
        fprintf(stderr, "Unknown type - only lib or sys");
        exit(1);
    }
}

int copy(char **args){
    char *file_name_src = args[2];
    char *file_name_dest = args[3];
    char *records_count = args[4];
    char *record_size = args[5];
    char *type = args[6];
    switch (check_type(type)) {
        case 0:
            return lib_copy(file_name_src, file_name_dest, atoi(records_count), atoi(record_size));
        case 1:
            return sys_copy(file_name_src, file_name_dest, atoi(records_count), atoi(record_size));
    }
    return 1;
}

int sort(char **args){
    char *file_name = args[2];
    char *records_count = args[3];
    char *record_size = args[4];
    char *type = args[5];
    switch (check_type(type)) {
        case 0:
            return lib_sort(file_name, atoi(records_count), atoi(record_size));
        case 1:
            return sys_sort(file_name, atoi(records_count), atoi(record_size));
    }
    return 1;
}

void check_arg_count(int argc, int desired){
    if(argc <= desired){
        fprintf(stderr, "Wrong arguments count");
        exit(1);
    }
}

int handle_command(int argc, char **argv) {
    check_arg_count(argc, 1);
    char *command = argv[1];
    if(strcmp(command, "generate") == 0){
        check_arg_count(argc, 4);
        generate_file(argv);
    } else if (strcmp(command, "sort") == 0) {
        check_arg_count(argc, 5);
        sort(argv);
    } else if (strcmp(command, "copy") == 0) {
        check_arg_count(argc, 6);
        copy(argv);
    } else {
        fprintf(stderr, "Unknown command");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    int err;
    if((err = handle_command(argc, argv)) != 0){
        fprintf(stderr, "Program failed to execute command");
        return err;
    }
    return 0;
}