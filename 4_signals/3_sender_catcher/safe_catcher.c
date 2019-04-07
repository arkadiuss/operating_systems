#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

int usr2_received = 0;
int usr1_count = 0;
static volatile pid_t sender_pid = -1;
Mode mode;

void handle_usr1(int sig, siginfo_t *info, void *context){
    usr1_count++;
    sender_pid = info->si_pid;
    send_by_mode(sender_pid, SIGUSR1, mode, info->si_value);
}

void handle_usr2(int sig){
    usr2_received = 1;
}

void send_signals(){
    union sigval sv;
    sv.sival_int = 0;
    printf("Sending back to: %d\n", sender_pid);
    for(int i = 0; i < usr1_count; i++){
        sv.sival_int = mode == QUEUE ? i+1 : 0;
        send_by_mode(sender_pid, SIGUSR1, mode, sv);
    }
    send_by_mode(sender_pid, SIGUSR2, mode, sv);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Wrong arguments count\n");
        return 1;
    }
    printf("Catcher started with PID: %d\n", getpid());
    struct sigaction act1, act2;
    set_def_sigaction(&act1);
    set_def_sigaction(&act2);
    act1.sa_flags = SA_SIGINFO;
    act1.sa_handler = handle_usr1;
    sigaction(SIGUSR1, &act1, NULL);
    sigaction(SIGRTMIN + 1, &act1, NULL);
    act2.sa_handler = handle_usr2;
    sigaction(SIGUSR2, &act2, NULL);
    sigaction(SIGRTMIN + 2, &act2, NULL);
    mode = get_mode(argv[1]);
    while(!usr2_received) { pause(); }
    printf("Received %d usr1 signals\n", usr1_count);
    /*if(sender_pid != -1)
        //send_signals();
    else
        fprintf(stderr, "No sender pid\n");*/
    return 0;
}
