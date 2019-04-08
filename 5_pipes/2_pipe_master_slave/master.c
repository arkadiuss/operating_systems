#include <stdio.h>
#include <sys-ops-commons.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const int READ_SIZE = 100;

int main(int argc, char **argv) {
    validate_argc(argc, 1);
    char *fifo_location = argv[1];
    if(mkfifo(fifo_location, 0666) == -1){
        fprintf(stderr, "Unable to create fifo\n");
    }
    int fifo_d;
    if((fifo_d = open(fifo_location, O_RDONLY)) == -1) {
        fprintf(stderr, "Unable to open fifo\n");
        return 1;
    }
    char buffer[READ_SIZE + 1];
    int read_size;
    while((read_size = read(fifo_d, buffer, READ_SIZE)) > 0){
        buffer[read_size] = '\0';
        printf("%s", buffer);
    }
    close(fifo_d);
    return 0;
}