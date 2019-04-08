#include <stdio.h>
#include <sys-ops-commons.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

const int WRITE_SIZE = 100;

void send_messages(int fd, int msg_num) {
    srand(time(0));
    for(int i = 0; i < msg_num; i++){
        FILE *date_out = popen("date", "r");
        char date[WRITE_SIZE], buff[WRITE_SIZE];
        int read_size = (int) fread(date, sizeof(char), 80, date_out);
        buff[read_size] = '\0';
        sprintf(buff, "%d: %s", getpid(), date);
        write(fd, buff, WRITE_SIZE);
        sleep(rand()%5+2);
    }
}

int main(int argc, char **argv) {
    validate_argc(argc, 2);
    if(!is_integer(argv[2])) return 1;
    int msg_num = atoi(argv[2]);
    int fd;
    if((fd = open(argv[1], O_WRONLY)) == -1){
        fprintf(stderr, "Unable to open fifo\n");
    }
    send_messages(fd, msg_num);
    return 0;
}