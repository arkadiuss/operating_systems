#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int blocked = 0;

void handle_int(int sig){
    printf("Received SIG_INT\n");
    exit(0);
}

void handle_stp(int sig){
    if(!blocked)
        printf("Type CTRL+Z to continue or CTRL+C to quit\n");
    else
        printf("Continuing\n");
    blocked = 1 - blocked;
}

void show_cur_date(){
    time_t ctime;
    time(&ctime);
    struct tm *time_info;
    time_info = localtime(&ctime);
    char buffer[20];
    strftime(buffer, 20, "%Y-%m-%d %H:%M", time_info);
    printf("Current time: %s\n", buffer);
}

void wait_for_signal(){
    /* ALTERNATIVE SOLUTION
    sigset_t mask;
    sigemptyset(&mask);
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGTSTP);
    sigsuspend(&mask);*/
    pause();
}

int main() {
    signal(SIGINT, handle_int);
    struct sigaction act;
    act.sa_handler = handle_stp;
    sigaction(SIGTSTP, &act, NULL);
    while(1) {
        if(blocked) {
            wait_for_signal();
        } else {
            show_cur_date();
        }
        sleep(1);
    }
    return 0;
}
