#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "common.h"

void send_signals(pid_t pid, int signals_num, Mode mode){
    union sigval sv;
    sv.sival_int = 0;
    while(signals_num--) {
        send_by_mode(pid, SIGUSR1, mode, sv);
    }
    send_by_mode(pid, SIGUSR2, mode, sv);
}

int usr2_resp_received = 0;
int usr1_resp_count = 0;
int max_usr1_num_received = 0;

void handle_usr1(int sig, siginfo_t *info, void *context) {
    if(info->si_code & SI_QUEUE){
        max_usr1_num_received = info->si_value.sival_int;
    }
    usr1_resp_count++;
}

void handle_usr2(int sig) {
    usr2_resp_received = 1;
}

int main(int argc, char **argv) {
    if(argc < 4) {
        fprintf(stderr, "Wrong arguments count\n");
        return 1;
    }
    printf("Sender PID: %d\n", getpid());

    pid_t catcher_pid = get_pid(argv[1]);
    int signals_number = get_signal_number(argv[2]);
    Mode mode = get_mode(argv[3]);

    struct sigaction act1, act2;
    sigemptyset (&act1.sa_mask);
    sigemptyset (&act2.sa_mask);
    act1.sa_flags = SA_SIGINFO;
    act2.sa_flags = 0;
    act1.sa_handler = handle_usr1;
    sigaction(SIGUSR1, &act1, NULL);
    sigaction(SIGRTMIN + 1, &act1, NULL);
    act2.sa_handler = handle_usr2;
    sigaction(SIGUSR2, &act2, NULL);
    sigaction(SIGRTMIN + 2, &act2, NULL);

    send_signals(catcher_pid, signals_number, mode);

    while(!usr2_resp_received){ pause(); }
    if(max_usr1_num_received != 0)
        printf("Received %d as a response with max value %d\n", usr1_resp_count, max_usr1_num_received);
    else
        printf("Received %d as a response\n", usr1_resp_count);
    return 0;
}