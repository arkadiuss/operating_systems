#include <stdio.h>
#include <stdlib.h>
#include <sys-ops-commons.h>

void execute_commands(const char *command){
    printf("Executing: %s", command);
}

int main(int argc, char **argv) {
    validate_argc(argc, 1);
    char *file_name = argv[1];
    char* buffer = malloc(sizeof(char)*1000);
    read_whole_file(file_name, buffer);
    execute_commands(buffer);
    free(buffer);
    return 0;
}