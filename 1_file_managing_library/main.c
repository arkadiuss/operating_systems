#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_managing_library.h"

char** file_table = NULL;
int TAB_SIZE = 10;

void print_files(char** files, int n){
    for(int i = 0; i < 1; i++) {
        if(files[i] != NULL)
            printf("%s", files[i]);
    }
}

void create_table(char **args, int i) {
    char *size = args[i + 1];
    TAB_SIZE = atoi(size);
    file_table = create_file_table(TAB_SIZE);
}

void search_directory(char **args, int i) {
    char* dir = args[i + 1];
    char* file_name = args[i + 2];
    char* tmp_file_name = args[i + 3];
    s_file file = get_current_location();
    set_location(&file, dir);
    set_file_name(&file, file_name);
    find_file(&file, tmp_file_name);
}

void insert_to_table(char **args, int i) {
    char* tmp_file_name = args[i + 3];
    insert_content_to_table(file_table, TAB_SIZE, tmp_file_name);
}

void remove_block(char **args, int i) {
    char *index = args[i + 1];
    int index_int = atoi(index);
    remove_file(file_table, index_int);
}

void exec_with_time(void (*op)(char**, int), char **args, int i) {
    op(args, i);
}

int main(int argc, char** argv) {
    int i = 1;
    while(i < argc) {
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
    print_files(file_table, TAB_SIZE);
    clear_table(file_table, TAB_SIZE);
    return 0;
}