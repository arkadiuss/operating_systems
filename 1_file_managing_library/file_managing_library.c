//
// Created by arkadius on 07.03.19.
//
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "file_managing_library.h"

const char *tmp_location = "/tmp";

s_file get_current_location() {
    s_file file;
    strcpy(file.location, ".");
    strcpy(file.file_name, "");
    return file;
}

void set_location(s_file *file, const char *location) {
    strcpy(file -> location, location);
}

void set_file_name(s_file *file, const char *file_name) {
    strcpy(file -> file_name, file_name);
}

char **create_file_table(int size) {
    return (char**) calloc(size, sizeof(char*));
}

int find_file(s_file *file, const char *tmp_file_name) {
    char command[512];
    sprintf(command, "find %s -name \"*%s*\" > %s/%s", file->location, file->file_name, tmp_location, tmp_file_name);
    return system(command);
}

int get_free_index(char **file_table, int file_tab_size) {
    for(int i = 0; i < file_tab_size; i++) {
        if(file_table[i] == NULL) return i;
    }
    fprintf(stderr, "No space left in the table\n");
    return -1;
}

long get_file_size(FILE *file) {
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return size;
}

char* read_file_content(FILE *file, long file_size) {
    char *file_content = (char*) malloc((size_t) (file_size + 1));
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
    char file_path[1024];
    sprintf(file_path, "%s/%s", tmp_location, file_name);
    if((file = fopen(file_path, "r")) == NULL){
        return NULL;
    }
    long file_size = get_file_size(file);
    char *file_content = read_file_content(file, file_size);
    fclose(file);
    return file_content;
}

int insert_content_to_table(char **file_table, int file_tab_size, const char *tmp_file_name) {
    if(file_table == NULL) {
        fprintf(stderr, "File table uninitialized\n");
        return -1;
    }
    int index;
    if((index = get_free_index(file_table, file_tab_size)) == -1){
        return -1;
    }
    if((file_table[index] = read_file(tmp_file_name)) == NULL){
        return -1;
    }
    return index;
}

int find_and_insert_named(char **file_table, int file_tab_size, s_file *file, const char *tmp_file_name) {
    int err;
    if((err = find_file(file, tmp_file_name)) == -1){
        return err;
    }
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
