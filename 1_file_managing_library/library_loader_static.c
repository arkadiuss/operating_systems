//
// Created by arkadius on 09.03.19.
//
#include "library_loader.h"

int load_library(fm_functions *functions) {
    functions->get_current_location = get_current_location;
    functions->set_file_name = set_file_name;
    functions->set_location = set_location;
    functions->create_file_table = create_file_table;
    functions->find_file = find_file;
    functions->insert_content_to_table = insert_content_to_table;
    functions->remove_file = remove_file;
    functions->find_and_insert = find_and_insert;
    functions->find_and_insert_named = find_and_insert_named;
    functions->clear_table = clear_table;
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
    return 0;
}
