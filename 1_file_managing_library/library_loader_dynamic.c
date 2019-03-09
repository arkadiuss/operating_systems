//
// Created by arkadius on 09.03.19.
//

#include <dlfcn.h>
#include "library_loader.h"

void load_library(fm_functions *functions) {
    void *handle = dlopen("./file_managing_library.so", RTLD_LAZY);
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
    dlclose(handle);
}

