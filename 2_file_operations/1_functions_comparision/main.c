#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "files_operations.h"

int generate_file(char **args){
    char *file_name = args[2];
    char *records_count = args[3];
    char *record_size = args[4];
    return generate(file_name, atoi(records_count), atoi(record_size));
}

int check_type(char *type) {
    if(strcmp(type, "lib") == 0) {
        return 0;
    } else if(strcmp(type, "sys") == 0){
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
    int records_count = atoi(args[3]);
    int record_size = atoi(args[4]);
    char *type = args[5];
    switch (check_type(type)) {
        case 0:
            return lib_sort(file_name, records_count, record_size);
        case 1:
            return sys_sort(file_name, records_count, record_size);
    }
    return 1;
}

void check_arg_count(int argc, int desired){
    if(argc <= desired){
        fprintf(stderr, "Wrong arguments count");
        exit(1);
    }
}

double timeval_diff(struct timeval t1, struct timeval t2) {
    return ((double) t2.tv_sec - t1.tv_sec) * 1000.0f + ((double) t2.tv_usec - t1.tv_usec) / 1000.0f;
}

int exec_with_time(int (*operation)(char**), char **args, const char *name) {
    struct timeval start, end;
    struct rusage d_start, d_end;
    getrusage(RUSAGE_SELF, &d_start);
    gettimeofday(&start, NULL);
    int err;
    if((err = operation(args)) == -1){
        return err;
    }
    getrusage(RUSAGE_SELF, &d_end);
    gettimeofday(&end, NULL);
    double real_ms = timeval_diff(start, end);
    double user_ms = timeval_diff(d_start.ru_utime, d_end.ru_utime);
    double system_ms = timeval_diff(d_start.ru_stime, d_end.ru_stime);
    printf("Operation %s time:\n user: %fms\n system: %fms\n real: %fms\n", name, user_ms, system_ms, real_ms);
    return 0;
}


int handle_command(int argc, char **argv) {
    check_arg_count(argc, 1);
    char *command = argv[1];
    if(strcmp(command, "generate") == 0){
        check_arg_count(argc, 4);
        exec_with_time(generate_file,argv,"generating");
    } else if (strcmp(command, "sort") == 0) {
        check_arg_count(argc, 5);
        exec_with_time(sort,argv,"sorting");
    } else if (strcmp(command, "copy") == 0) {
        check_arg_count(argc, 6);
        exec_with_time(copy,argv,"copying");
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