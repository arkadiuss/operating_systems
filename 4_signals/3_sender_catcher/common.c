//
// Created by arkadius on 30.03.19.
//
#define _XOPEN_SOURCE 500
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>

int is_integer(char* text){
    int i = 0;
    char c;
    while((c = text[i++]) != '\0'){
        if(c < 48 || c > 57) {
            fprintf(stderr, "%s in not an integer\n", text);
            return 0;
        }
    }
    return 1;
}

pid_t get_pid(char *pid) {
    if(!is_integer(pid)) {
        fprintf(stderr, "Wrong number of signals\n");
        exit(1);
    }
    pid_t p = (pid_t) atoi(pid);
    if(kill(p, 0) != 0){
        fprintf(stderr, "There is no process with this PID\n");
        exit(1);
    }
    return p;
}

int get_signal_number(char *number) {
    if(!is_integer(number)) {
        fprintf(stderr, "Wrong number of signals\n");
        exit(1);
    }
    return atoi(number);
}

Mode get_mode(char *mode){
    if(strcmp(mode, "kill") == 0) {
        return KILL;
    } else if(strcmp(mode, "queue") == 0) {
        return QUEUE;
    } else if(strcmp(mode, "rt") == 0) {
        return RT;
    } else {
        fprintf(stderr, "Unknown mode\n");
        exit(1);
    }
}

void set_def_sigaction(struct sigaction* act){
    sigfillset (&act->sa_mask);
    sigdelset(&act->sa_mask, SIGUSR1);
    sigdelset(&act->sa_mask, SIGUSR2);
    act->sa_flags = 0;
}

void send_by_mode(pid_t pid, int sig, Mode mode, union sigval data) {
    switch (mode){
        case KILL:
            kill(pid, sig);
            break;
        case QUEUE:
            sigqueue(pid, sig, data);
            break;
        case RT:
            if(sig == SIGUSR1)
                kill(pid, SIGRTMIN + 1);
            else if(sig == SIGUSR2)
                kill(pid, SIGRTMIN + 2);
            else
                kill(pid, sig);
            break;
    }
}
