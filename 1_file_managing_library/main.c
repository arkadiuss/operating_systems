#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "library_loader.h"

char** file_table = NULL;
int TAB_SIZE = 10;
fm_functions functions;

void print_files(char** files, int n){
    for(int i = 0; i < 1; i++) {
        if(files[i] != NULL)
            printf("%s", files[i]);
    }
}

void create_table(char **args, int i) {
    char *size = args[i + 1];
    //TODO: validation is int
    TAB_SIZE = atoi(size);
    file_table = functions.create_file_table(TAB_SIZE);
}

void search_directory(char **args, int i) {
    char* dir = args[i + 1];
    char* file_name = args[i + 2];
    char* tmp_file_name = args[i + 3];
    s_file file = functions.get_current_location();
    functions.set_location(&file, dir);
    functions.set_file_name(&file, file_name);
    functions.find_file(&file, tmp_file_name);
}

void insert_to_table(char **args, int i) {
    char* tmp_file_name = args[i + 3];
    functions.insert_content_to_table(file_table, TAB_SIZE, tmp_file_name);
}

void remove_block(char **args, int i) {
    char *index = args[i + 1];
    int index_int = atoi(index);
    functions.remove_file(file_table, index_int);
}

double timeval_diff(struct timeval t1, struct timeval t2) {
    return ((double) t2.tv_sec - t1.tv_sec) * 1000.0f + ((double) t2.tv_usec - t1.tv_usec) / 1000.0f;
}

void exec_with_time(void (*op)(char**, int), char **args, int i) {
    struct timeval start, end;
    struct rusage d_start, d_end;
    getrusage(RUSAGE_SELF, &d_start);
    gettimeofday(&start, NULL);
    //TODO: handle return codes
    op(args, i);
    getrusage(RUSAGE_SELF, &d_end);
    gettimeofday(&end, NULL);
    double real_ms = timeval_diff(start, end);
    double user_ms = timeval_diff(d_start.ru_utime, d_end.ru_utime);
    double system_ms = timeval_diff(d_start.ru_stime, d_end.ru_stime);
    printf("Operation time:\n user: %fms\n system: %fms\n real: %fms\n", user_ms, system_ms, real_ms);
}

int main(int argc, char** argv) {
    load_library(&functions);
    int i = 1;
    while(i < argc) {
        //TODO: validation of number of argument
        char* command = argv[i];
        if(strcmp(command, "create_table") == 0){
            exec_with_time(create_table, argv, i);
            i = i + 2;
        } else if(strcmp(command, "search_directory") == 0) {
            exec_with_time(search_directory, argv, i);
            exec_with_time(insert_to_table, argv, i);
            i = i + 4;
        } else if(strcmp(command, "remove_block") == 0) {
            exec_with_time(remove_block, argv, i);
            i = i + 2;
        } else {
            fprintf(stderr, "Unknown command");
            return 1;
        }
    }
    //print_files(file_table, TAB_SIZE);
    functions.clear_table(file_table, TAB_SIZE);
    return 0;
}