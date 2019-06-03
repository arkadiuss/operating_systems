#ifndef SYS_OPS_COMMONS_LIBRARY_H
#define SYS_OPS_COMMONS_LIBRARY_H
#include <stdio.h>

void validate_argc(int argc, int required);
long get_file_size(const char *file_name);
long read_whole_file(const char *file_name, char *buffer);
int is_integer(char *str);
int is_integer_list(char* str);
int as_integer(char *str);
void show_error_and_exit(const char *err, int exit_code);
int split_to_arr(char **args, char *msg);
int min(int a, int b);
int max(int a, int b);
#endif