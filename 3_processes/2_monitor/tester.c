#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <zconf.h>
#include <string.h>

//
// Created by arkadius on 23.03.19.
//
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

void validate_args(int argc, char **argv) {
    if(argc < 5) {
        fprintf(stderr, "Wrong arguments count");
        exit(1);
    }

    if(!is_integer(argv[2])||!is_integer(argv[3])||!is_integer(argv[4])){
        exit(1);
    }

    if(strcmp(argv[2], argv[3]) == 1) {
        fprintf(stderr, "pmin should be greater than pmax");
    }
}

int get_rand(int a, int b){
    return rand()%(b - a + 1) + a;
}

char *get_date(){
    struct tm * timeinfo;
    time_t vtime;
    time(&vtime);
    timeinfo = localtime ( &vtime );
    char *buff = malloc(sizeof(char) * 20);
    strftime(buff, 20, "%Y-%m-%d_%H-%M-%S", timeinfo);
    return buff;
}

char *get_random_bytes(int n) {
    char *bytes = malloc(sizeof(char)*n+1);
    for(int i = 0; i < n; i++){
        bytes[i] = (char) get_rand(97, 122);
    }
    bytes[n] = '\0';
    return bytes;
}

void append_to_file(const char *file_name, int time, int bytes){
    char line[1024];
    pid_t pid = getpid();
    FILE *file;
    char *date = get_date();
    char *rand_bytes = get_random_bytes(bytes);
    sprintf(line, "%d %d %s %s\n", pid, time, date, rand_bytes);
    free(rand_bytes);
    free(date);
    if((file = fopen(file_name, "a")) != NULL) {
        fwrite(line, sizeof(char), strlen(line), file);
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open requested file");
        exit(1);
    }
}

int main(int argc, char **argv){
    validate_args(argc, argv);
    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    int bytes = atoi(argv[4]);
    srand(time(NULL));
    while(1) {
        int time = get_rand(pmin, pmax);
        append_to_file(argv[1], time, bytes);
        fflush(stdout);
        sleep(time);
    }
}