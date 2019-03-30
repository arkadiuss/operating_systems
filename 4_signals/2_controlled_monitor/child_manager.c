//
// Created by arkadius on 30.03.19.
//

#include "child_manager.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <wait.h>
#include <sys/resource.h>

child** create_children_table(size_t size) {
    return calloc(sizeof(child*), size + 1);
}

child* create_child(const char *file_name, pid_t pid) {
    child* child = malloc(sizeof(child));
    child->file_name = strdup(file_name);
    child->pid = pid;
    child->status = RUNNING;
    return child;
}

void list_processes(child** children) {
    int i = 0;
    printf("PROCESSES LIST: \n");
    while(children[i] != NULL) {
        printf("PROCESS %d file %s status %d\n", children[i]->pid, children[i]->file_name, children[i]->status);
        i++;
    }
    printf("--------------\n");
}

child* get_child_by_pid(child **children, pid_t pid) {
    int i = 0;
    while(children[i] != NULL) {
        if(children[i]->pid == pid)
            return children[i];
        i++;
    }
    return NULL;
}

void start_process(child *child) {
    if(child != NULL && child->status == RUNNING) {
        if(kill(child->pid, 0) == 0) {
            kill(child->pid, SIGCONT);
            child->status = STOPPED;
        }
    }
}

void start_all_processes(child **children) {
    int i = 0;
    while(children[i] != NULL){
        start_process(children[i]);
        i++;
    }
}

void stop_process(child *child) {
    if(child != NULL && child->status == RUNNING) {
        if(kill(child->pid, 0) == 0) {
            kill(child->pid, SIGTSTP);
            child->status = STOPPED;
        }
    }
}

void stop_all_processes(child **children) {
    int i = 0;
    while(children[i] != NULL){
        stop_process(children[i]);
        i++;
    }
}

double to_milis(struct timeval tm) {
    return ((double) tm.tv_usec)/1000.0 + tm.tv_sec*1000.0;
}

void end_processes(child **children) {
    struct rusage usage;
    double sys_sum = 0;
    double usr_sum = 0;
    int status;
    int i = 0;
    while(children[i] != NULL){
        kill(children[i]->pid, SIGINT);
        waitpid(children[i]->pid, &status, 0);
        getrusage(RUSAGE_CHILDREN, &usage);
        printf("STATUS of %d is %d usage: maxrss %ld utime: %.3f stime: %.3f\n", i,
                status, usage.ru_maxrss, to_milis(usage.ru_utime) - usr_sum,
               to_milis(usage.ru_stime) - sys_sum);
        sys_sum += to_milis(usage.ru_stime);
        usr_sum += to_milis(usage.ru_utime);
        children[i] = ENDED;
        i++;
    }
}

void clear_children(child **children) {
    int i = 0;
    while(children[i] != NULL){
        free(children[i]->file_name);
        free(children[i]);
        i++;
    }
    free(children);
}