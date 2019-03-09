#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_managing_library.h"

char** file_table = NULL;
const int TAB_SIZE = 10;

void print_files(char** files, int n){
    for(int i = 0; i < 1; i++) {
        if(files[i] != NULL)
            printf("%s", files[i]);
    }
}

void create_table(char *size) {
    int t_size = strtol(size);
    file_table = create_file_table(t_size);
}

void search_directory(char *dir, char *file_name, char *tmp_file_name) {
    s_file file = get_current_location();
    set_location(&file, dir);
    set_file_name(&file, file_name);
    find_file(&file, tmp_file_name);
    insert_content_to_table(file_table, TAB_SIZE, tmp_file_name);
}

void remove_block(char *index) {
    int i = atoi(index);
    remove_file(file_table, i);
}

int main(int argc, char** argv) {
    int i = 1;
    while(i < argc) {
        char* command = argv[i];
        if(strcmp(command, "create_table") == 0){
            create_table(argv[i + 1]);
            i = i + 2;
        } else if(strcmp(command, "search_directory") == 0) {
            search_directory(argv[i + 1], argv[i + 2], argv[i + 3]);
            i = i + 4;
        } else if(strcmp(command, "remove_block") == 0) {
            remove_block(argv[i + 1]);
            i = i + 2;
        } else {
            return 1;
        }
    }
    print_files(file_table, TAB_SIZE);
    clear_table(file_table, TAB_SIZE);
    return 0;
}