//
// Created by arkadius on 30.03.19.
//

#ifndef CONTROLLEDMONITOR_CHILD_MANAGER_H
#define CONTROLLEDMONITOR_CHILD_MANAGER_H

#include <stdlib.h>
#include <sys/types.h>

typedef enum {
    RUNNING = 0, STOPPED = 1, ENDED = 2
} child_status;

typedef struct {
    pid_t pid;
    child_status status;
    char *file_name;
} child;

child** create_children_table(size_t size);
child* create_child(const char *file_name, pid_t pid);
void list_processes(child** children);
child* get_child_by_pid(child **children, int pid);
void start_process(child *child);
void start_all_processes(child **children);
void stop_process(child *child);
void stop_all_processes(child **children);
void end_processes(child **children);
void clear_children(child **children);

#endif //CONTROLLEDMONITOR_CHILD_MANAGER_H
