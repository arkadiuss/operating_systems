#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int child_started = 0;
pid_t child_pid = -1;

void start_child(){
    printf("Starting child process\n");
    int pid;
    child_started = 1;
    if((pid = fork()) == 0) {
        execl("./date_dispatcher.sh", "./date_dispatcher.sh", NULL);
        exit(1);
    } else {
        child_pid = pid;
    }
}

void stop_child(){
    if(child_started && child_pid != -1) {
        printf("Stopping child process PID: %d\n", child_pid);
        kill(child_pid, SIGKILL);
        child_started = 0;
        child_pid = -1;
    }
}

void handle_int(int sig){
    printf("Received SIG_INT\n");
    stop_child();
    exit(0);
}

void handle_stp(int sig){
    if(!child_started)
        start_child();
    else
        stop_child();
}

int main() {
    signal(SIGINT, handle_int);
    struct sigaction action;
    action.sa_handler = handle_stp;
    sigaction(SIGTSTP, &action, NULL);
    printf("Process PID: %d\n", getpid());
    start_child();
    while(1) { pause(); }
    return 0;
}