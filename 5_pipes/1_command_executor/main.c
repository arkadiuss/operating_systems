#include <stdio.h>
#include <stdlib.h>
#include <sys-ops-commons.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

const int MAX_ARGS;
const char* COMMAND_DELIMITER = "|";
const char* ARGS_DELIMITER = " ";

void execute(char *command_with_args){
    char *args[MAX_ARGS];
    char *command;
    char *ptr = strtok(command_with_args, ARGS_DELIMITER);
    command = ptr;
    int i = 0;
    args[i] = ptr;
    while(ptr != NULL) {
        ptr = strtok(NULL, ARGS_DELIMITER);
        args[++i] = ptr;
    }
    execvp(command, args);
    fprintf(stderr, "Sth went wrong\n");
    exit(1);
}

int redirect_and_execute(char *command, int prev_fd) {
    int fd[2];
    pipe(fd);
    if(fork() == 0) {
        close(fd[0]);
        if(prev_fd != -1) {
            dup2(prev_fd, STDIN_FILENO);
            close(prev_fd);
        }
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execute(command);
        exit(0);
    }
    close(prev_fd);
    close(fd[1]);
    return fd[0];
}

void execute_commands(char *command){
    char *ptr = strtok(command, COMMAND_DELIMITER);
    int prev_fd = -1;
    int commands_num = 0;
    while(ptr != NULL) {
        prev_fd = redirect_and_execute(ptr, prev_fd);
        ptr = strtok(NULL, COMMAND_DELIMITER);
        commands_num++;
    }
    char buffer[101];
    int read_size = 0;
    while((read_size = read(prev_fd, buffer, 100)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);
    }
    while(commands_num--){
        wait(NULL);
    }
    close(prev_fd);
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