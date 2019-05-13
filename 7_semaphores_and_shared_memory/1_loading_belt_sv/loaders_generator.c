#include <stdio.h>
#include <sys-ops-commons.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <wait.h>

#define MAX_LOADERS 100

int loaders[MAX_LOADERS];
int L;
void start_loaders(int L, int N){
    int i = 0;
    while(i < L) {
        int n = rand()%N + 1;
        int pid;
        if((pid = fork()) == 0) {
            char nstr[9];
            sprintf(nstr, "%d", n);
            int c = rand()%20;
            if(c > 10) {
                char cstr[9];
                sprintf(cstr, "%d", c);
                execl("./loader", "./loader", nstr, cstr, NULL);
                show_error_and_exit("Unable to start loader", 1);
            } else {
                execl("./loader", "./loader", nstr, NULL);
                show_error_and_exit("Unable to start loader", 1);
            }
        } else {
            loaders[i++] = pid;
        }
    }
}

void stop_loaders(int signum) {
    for(int i = 0; i < L; i++) {
        kill(loaders[i], SIGINT);
    }
    exit(0);
}

int main(int argc, char **argv) {
    validate_argc(argc, 2);
    L = as_integer(argv[1]);
    int N = as_integer(argv[2]);

    struct sigaction intact;
    sigfillset(&intact.sa_mask);
    intact.sa_handler = stop_loaders;
    intact.sa_flags = 0;
    sigaction(SIGINT, &intact, NULL);

    srand(time(0));
    start_loaders(L, N);
    while(L--){
        wait(NULL);
    }
    return 0;
}