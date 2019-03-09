//
// Created by arkadius on 07.03.19.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "file_managing_library.h"

s_file get_current_location() {
    s_file file;
    strcpy(file.location, ".");
    strcpy(file.file_name, "");
    return file;
}

int set_location(s_file *file, const char *location) {
    strcpy(file -> location, location);
    return 0;
}

int set_file_name(s_file *file, const char *file_name) {
    strcpy(file -> file_name, file_name);
    return 0;
}

char **create_file_table(int size) {
    return (char**) calloc(size, sizeof(char*));
}
//TODO: validation
int find_file(s_file *file, const char *tmp_file_name) {
    char command[512];
    sprintf(command, "find %s -name \"*%s*\" > %s", file->location, file->file_name, tmp_file_name);
    system(command);
}

int get_free_index(char **file_table, int file_tab_size) {
    for(int i = 0; i < file_tab_size; i++) {
        if(file_table[i] == NULL) return i;
    }
    return -1;
}

long get_file_size(FILE *file) {
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
}

char* read_file_content(FILE *file, long file_size) {
    char *file_content = (char*) malloc(file_size);
    int character;
    int n = 0;
    while((character = fgetc(file)) != EOF){
        file_content[n++] = (char) character;
    }
    file_content[n] = '\0';
    return file_content;
}
//TODO: too big files
char* read_file(const char* file_name) {
    FILE *file;
    if((file = fopen(file_name, "r")) == NULL){
        return NULL;
    }
    long file_size = get_file_size(file);
    char *file_content = read_file_content(file, file_size);
    fclose(file);
    return file_content;
}

int insert_content_to_table(char **file_table, int file_tab_size, const char *tmp_file_name) {
    int index;
    if((index = get_free_index(file_table, file_tab_size)) != 0){
        return -1;
    }
    file_table[index] = read_file(tmp_file_name);
    return index;
}

int find_and_insert_named(char **file_table, int file_tab_size, s_file *file, const char *tmp_file_name) {
    find_file(file, tmp_file_name);
    return insert_content_to_table(file_table, file_tab_size, tmp_file_name);
}

//TODO: dynamic file name
int find_and_insert(char **file_table, int file_tab_size, s_file *file) {
    return find_and_insert_named(file_table, file_tab_size, file, "tmp_file");
}

void remove_file(char **file_table, int index) {
    free(file_table[index]);
    file_table[index] = NULL;
}

void clear_table(char **file_table, int file_tab_size) {
    if(file_table != NULL) {
        for (int i = 0; i < file_tab_size; i++) {
            remove_file(file_table, i);
        }
        free(file_table);
    }
}
