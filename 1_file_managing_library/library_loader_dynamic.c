//
// Created by arkadius on 09.03.19.
//

#include <dlfcn.h>
#include <stdio.h>
#include "library_loader.h"

int load_library(fm_functions *functions) {
    void *handle = dlopen("./file_managing_library.so", RTLD_LAZY);
    if(!handle) {
        fprintf(stderr, "Unable to load file_managing_library");
        return 1;
    }
    functions->get_current_location = dlsym(handle, "get_current_location");
    functions->set_file_name = dlsym(handle, "set_file_name");
    functions->set_location = dlsym(handle, "set_location");
    functions->create_file_table = dlsym(handle, "create_file_table");
    functions->find_file = dlsym(handle, "find_file");
    functions->insert_content_to_table = dlsym(handle, "insert_content_to_table");
    functions->remove_file = dlsym(handle, "remove_file");
    functions->find_and_insert = dlsym(handle, "find_and_insert");
    functions->find_and_insert_named = dlsym(handle, "find_and_insert_named");
    functions->clear_table = dlsym(handle, "clear_table");
    functions->handle = handle;
    char* error;
    if((error = dlerror()) != NULL) {
        fprintf(stderr, "Errored: %s", error);
        fflush(stderr);
        return 1;
    }
    return 0;
}


int unload_library(fm_functions *functions) {
    functions->get_current_location = NULL;
    functions->set_file_name = NULL;
    functions->set_location = NULL;
    functions->create_file_table = NULL;
    functions->find_file = NULL;
    functions->insert_content_to_table = NULL;
    functions->remove_file = NULL;
    functions->find_and_insert = NULL;
    functions->find_and_insert_named = NULL;
    functions->clear_table = NULL;
    return dlclose(functions->handle);
}

