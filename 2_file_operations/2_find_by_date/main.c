#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "file_finder.h"

int lower(time_t t1, time_t t2) {
    return t1 < t2;
}

int equal(time_t t1, time_t t2) {
    long long to_days = 60LL*60LL*24LL + 7200;
    return abs(t1 - t2)/to_days == 0;
}

int greater(time_t t1, time_t t2) {
    return t1 > t2;
}

int (*get_command(char *command_str))(time_t,time_t) {
    if(strcmp(command_str, "<") == 0) {
        return lower;
    } else if(strcmp(command_str, "=") == 0) {
        return equal;
    } else if(strcmp(command_str, ">") == 0) {
        return greater;
    } else {
        fprintf(stderr, "Unknown operator");
        exit(1);
    }
}

time_t get_time(char *time_str) {
    struct tm tm;
    if(strptime(time_str, "%d-%m-%Y", &tm) == NULL) {
        fprintf(stderr, "Incorrect format of date");
        exit(1);
    }
    return mktime(&tm);
}

int main(int argc, char **argv) {
    if(argc < 4){
        fprintf(stderr, "Wrong arguments count");
        return 1;
    }
    char path[1024];
    realpath(argv[1], path);
    int (*compare)(time_t, time_t) = get_command(argv[2]);
    time_t time = get_time(argv[3]);
    find_with_nftw(path, time, compare);
    return 0;
}