//
// Created by arkadius on 09.03.19.
//

#ifndef INC_1_FILE_MANAGING_LIBRARY_LIBRARY_LOADER_H
#define INC_1_FILE_MANAGING_LIBRARY_LIBRARY_LOADER_H

#include "file_managing_library.h"

typedef struct fm_functions {
    s_file (*get_current_location)();
    void (*set_location)(s_file*, const char*);
    void (*set_file_name)(s_file*, const char*);

    char** (*create_file_table)(int);
    int (*find_file)(s_file *file, const char*);
    int (*insert_content_to_table)(char **, int, const char *);
    int (*find_and_insert)(char**, int, s_file*);
    int (*find_and_insert_named)(char**, int, s_file*, const char*);
    void (*remove_file)(char**, int);
    void (*clear_table)(char**, int);

    void* handle;
} fm_functions;

int load_library(fm_functions *functions);
int unload_library(fm_functions *functions);
#endif //INC_1_FILE_MANAGING_LIBRARY_LIBRARY_LOADER_H
