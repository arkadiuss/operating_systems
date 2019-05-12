#include <stdio.h>
#include <sys-ops-commons.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

void start_loaders(int L, int N){
    while(L--) {
        int n = rand()%N + 1;
        if(fork() == 0) {
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
        }
    }
}

int main(int argc, char **argv) {
    validate_argc(argc, 2);
    int L = as_integer(argv[1]);
    int N = as_integer(argv[2]);
    srand(time(0));
    start_loaders(L, N);
    return 0;
}