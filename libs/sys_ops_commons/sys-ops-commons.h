#ifndef SYS_OPS_COMMONS_LIBRARY_H
#define SYS_OPS_COMMONS_LIBRARY_H
#include <stdio.h>

void validate_argc(int argc, int required);
size_t get_file_size(const char *file_name);
size_t read_whole_file(const char *file_name, char *buffer);
int is_integer(char *str);
void show_error_and_exit(const char *err, int exit_code);
#endif