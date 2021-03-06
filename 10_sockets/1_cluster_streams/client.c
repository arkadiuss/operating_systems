#include <stdio.h>
#include <sys/socket.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define SINT16_T sizeof(uint16_t)

char name[NAME_SIZE];
client_type type;
int sock;

void init_socket(int argc, char ** argv){
    if(type == LOCAL) {
        const char *path = argv[3];
        if((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            show_error_and_exit("Unable to create socket", 1);
        }
        struct sockaddr_un address;
        address.sun_family = AF_UNIX;
        strcpy(address.sun_path, path);
        if(connect(sock, (struct sockaddr*) &address, sizeof(address)) < 0) {
            show_error_and_exit("Unable to connect to server", 1);
        }
    } else if(type == REMOTE) {
        validate_argc(argc, 4);
        in_addr_t ip = inet_addr(argv[3]);
        int port = as_integer(argv[4]);
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            show_error_and_exit("Unable to create socket", 1);
        }
        struct sockaddr_in address;
        memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = ip;
        address.sin_port = htons(port);
        if(connect(sock, (struct sockaddr*) &address, sizeof(address)) < 0) {
            show_error_and_exit("Unable to connect to server", 1);
        }
    }
    uint8_t type = REGISTER;
    WRITE_OR_RETURN(sock, &type, TYPE_SIZE)
    uint16_t size = (uint16_t) strlen(name);
    WRITE_OR_RETURN(sock, &size, MSG_SIZE_SIZE)
    WRITE_OR_RETURN(sock, name, size)
    printf("Connected\n");
}



void count_words_and_respond() {
    uint16_t msg_size;
    READ_OR_RETURN(sock, &msg_size, MSG_SIZE_SIZE)
    char *text = malloc(sizeof(char)*msg_size);
    READ_OR_RETURN(sock, text, msg_size);
    printf("TEXT: %s\n", text);
    char command[MAX_FILE_SIZE];
    sprintf(command, "echo \"%s\" | tr -s ' ' '\\n' | sort | uniq -c", text);
    FILE *fres = popen(command, "r");
    char result[MAX_FILE_SIZE];
    if(fread(result, sizeof(char), MAX_FILE_SIZE, fres) <= 0) {
        fprintf(stderr, "Unable to read response\n");
        return;
    }
    free(text);
    message_type type = RESULT;
    uint16_t size = (uint16_t) strlen(result);
    WRITE_OR_RETURN(sock, &type, TYPE_SIZE);
    WRITE_OR_RETURN(sock, &size, MSG_SIZE_SIZE);
    WRITE_OR_RETURN(sock, result, size)
}

void respond_to_ping() {
    uint8_t type = PONG;
    WRITE_OR_RETURN(sock, &type, TYPE_SIZE)
}

void unregister(){
    uint8_t type = UNREGISTER;
    WRITE_OR_RETURN(sock, &type, TYPE_SIZE)
}

void handle_requests() {
    while (1) {
        uint8_t type;
        READ_OR_RETURN(sock, &type, TYPE_SIZE)
        printf("GOT MESSAGE \n");
        printf("TYPE %d\n", type);
        switch (type) {
            case COUNT:
                count_words_and_respond();
                break;
            case PING:
                respond_to_ping();
                break;
        }
    }
}

void int_handler(int sig) {
    unregister();
    close(sock);
    exit(0);
}

int main(int argc, char ** argv) {
    validate_argc(argc, 3);
    strcpy(name, argv[1]);
    type = get_type(argv[2]);
    init_socket(argc, argv);

    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = int_handler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    handle_requests();

    return 0;
}