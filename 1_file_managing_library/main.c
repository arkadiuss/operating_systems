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

int create_table(char **args, int i) {
    if(file_table != NULL) {
        fprintf(stderr, "Already initialized!\n");
        return -1;
    }
    char *size = args[i + 1];
    if(!is_integer(size))
        return -1;
    TAB_SIZE = atoi(size);
    file_table = functions.create_file_table(TAB_SIZE);
    return 0;
}

int search_directory(char **args, int i) {
    char* dir = args[i + 1];
    char* file_name = args[i + 2];
    char* tmp_file_name = args[i + 3];
    s_file file = functions.get_current_location();
    functions.set_location(&file, dir);
    functions.set_file_name(&file, file_name);
    return functions.find_file(&file, tmp_file_name);
}

int insert_to_table(char **args, int i) {
    char* tmp_file_name = args[i + 3];
    return functions.insert_content_to_table(file_table, TAB_SIZE, tmp_file_name);
}

int remove_block(char **args, int i) {
    char *index = args[i + 1];
    if(!is_integer(index))
        return 1;
    int index_int = atoi(index);
    functions.remove_file(file_table, index_int);
    return 0;
}

double timeval_diff(struct timeval t1, struct timeval t2) {
    return ((double) t2.tv_sec - t1.tv_sec) * 1000.0f + ((double) t2.tv_usec - t1.tv_usec) / 1000.0f;
}

int exec_with_time(int (*op)(char**, int), char **args, int i, const char *name) {
    struct timeval start, end;
    struct rusage d_start, d_end;
    getrusage(RUSAGE_SELF, &d_start);
    gettimeofday(&start, NULL);
    int err;
    if((err = op(args, i)) == -1){
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

void validate_args_num(int argc, int i, int c) {
    if(i + c > argc) {
        fprintf(stderr, "Unexpected number of arguments\n");
        exit(1);
    }
}

int process_commands(char** argv, int argc) {
    int i = 1;
    int err;
    while(i < argc) {
        char* command = argv[i];
        // for operations arguments have different meaning, not argv and argc, but argv and starting index
        if(strcmp(command, "create_table") == 0){
            validate_args_num(argc, i, 2);
            if((err = exec_with_time(create_table, argv, i, "creating table")) != 0){
                return err;
            }
            i = i + 2;
        } else if(strcmp(command, "search_directory") == 0) {
            validate_args_num(argc, i, 4);
            if((err = exec_with_time(search_directory, argv, i, "searching directory")) != 0){
                return err;
            }
            if((err = exec_with_time(insert_to_table, argv, i, "inserting to table")) != 0){
                return err;
            }
            i = i + 4;
        } else if(strcmp(command, "remove_block") == 0) {
            validate_args_num(argc, i, 2);
            if((err = exec_with_time(remove_block, argv, i, "removing block")) != 0){
                return err;
            }
            i = i + 2;
        } else {
            fprintf(stderr, "Unknown command\n");
            return -1;
        }
    }
    functions.clear_table(file_table, TAB_SIZE);
    return 0;
}

int main(int argc, char** argv) {
    load_library(&functions);
    int err;
    // for process_commands arguments are argv and argc
    // for operations arguments have different meanings
    if((err = exec_with_time(process_commands, argv, argc, "total")) == -1){
        fprintf(stderr, "Error while processing\n");
        return err;
    }
    unload_library(&functions);
    return 0;
}