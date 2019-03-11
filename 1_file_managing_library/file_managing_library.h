//
// Created by arkadius on 07.03.19.
//

#ifndef INC_1_FILE_MANAGING_LIBRARY_FILE_MANAGING_LIBRARY_H
#define INC_1_FILE_MANAGING_LIBRARY_FILE_MANAGING_LIBRARY_H

typedef struct file {
    char location[512];
    char file_name[64];
} s_file;

s_file get_current_location();
void set_location(s_file * file, const char* location);
void set_file_name(s_file * file, const char* location);

char** create_file_table(int size);
int find_file(s_file *file, const char *tmp_file_name);
int insert_content_to_table(char **file_table, int file_tab_size, const char *tmp_file_name);
int find_and_insert(char** file_table, int file_tab_size, s_file* file);
int find_and_insert_named(char** file_table, int file_tab_size, s_file* file, const char* tmp_file_name);
void remove_file(char** file_table, int index);
void clear_table(char** file_table, int file_tab_size);

#endif //INC_1_FILE_MANAGING_LIBRARY_FILE_MANAGING_LIBRARY_H
