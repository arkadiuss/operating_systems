#include <stdio.h>
#include <sys/socket.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>



char name[NAME_SIZE];
client_type type;
int sock;
struct sockaddr *serv_addr;
long addr_size;

#define READ_OR_RETURN(sock, from, buf) if(recvfrom(sock, buf, sizeof(message), 0, (struct sockaddr *) from, addr_size) != sizeof(message)) { fprintf(stderr, "Unable to read message\n Error: %s \n", strerror(errno)); return; }
#define SEND_OR_RETURN(sock, to, buf) if(sendto(sock, buf, sizeof(message), 0, to, addr_size) != sizeof(message)) { fprintf(stderr, "Unable to write message \n"); return; }


void init_socket(int argc, char ** argv){
    if(type == LOCAL) {
        const char *path = argv[3];
        if((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
            show_error_and_exit("Unable to create socket", 1);
        }
        struct sockaddr_un client_address;
        client_address.sun_family = AF_UNIX;
        sprintf(client_address.sun_path, "%s_%s", path, name);

        if(bind(sock, &client_address, sizeof(struct sockaddr_un)) < 0) {
            show_error_and_exit("Unable to bind to local socket", 1);
        }

        serv_addr = malloc(sizeof(struct sockaddr_un));
        addr_size = sizeof(struct sockaddr_un);
        struct sockaddr_un *address = (struct sockaddr_un *) serv_addr;
        address->sun_family = AF_UNIX;
        strcpy(address->sun_path, path);

        if(connect(sock, address, sizeof(struct sockaddr_un)) < 0) {
            show_error_and_exit("Unable to connect to server", 1);
        }
    } else if(type == REMOTE) {
        validate_argc(argc, 4);
        in_addr_t ip = inet_addr(argv[3]);
        int port = as_integer(argv[4]);
        if((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            show_error_and_exit("Unable to create socket", 1);
        }

        serv_addr = malloc(sizeof(struct sockaddr_in));
        addr_size = sizeof(struct sockaddr_in);
        struct sockaddr_in *address = (struct sockaddr_in*) serv_addr;
        memset(&serv_addr, 0, sizeof(struct sockaddr_in));
        address->sin_family = AF_INET;
        address->sin_addr.s_addr = ip;
        address->sin_port = htons(port);
        if(connect(sock, address, sizeof(struct sockaddr_in)) < 0) {
            show_error_and_exit("Unable to connect to server", 1);
        }
    }
    message msg;
    msg.type = REGISTER;
    msg.msg_len = strlen(name);
    strcpy(msg.msg, name);
    SEND_OR_RETURN(sock, serv_addr, &msg)
    printf("Connected\n");
}



void count_words_and_respond(message *msg) {
    printf("TEXT: %s\n", msg->msg);
    char command[MAX_FILE_SIZE];
    sprintf(command, "echo \"%s\" | tr -s ' ' '\\n' | sort | uniq -c", msg->msg);
    FILE *fres = popen(command, "r");
    message msg_res;
    msg_res.type = RESULT;
    if(fread(msg_res.msg, sizeof(char), MAX_FILE_SIZE, fres) <= 0) {
        fprintf(stderr, "Unable to read response\n");
        return;
    }
    msg_res.msg_len = strlen(msg_res.msg);
    SEND_OR_RETURN(sock, serv_addr, &msg_res)
}

void respond_to_ping() {
    message msg_res;
    msg_res.type = PONG;
    printf("Sending pong!\n");
    SEND_OR_RETURN(sock, serv_addr, &msg_res)
}

void unregister(){
    message msg_res;
    msg_res.type = UNREGISTER;
    SEND_OR_RETURN(sock, serv_addr, &msg_res)
}

void handle_requests() {
    while (1) {
        message msg;
        READ_OR_RETURN(sock, NULL, &msg)
        printf("GOT MESSAGE \n");
        printf("TYPE %d\n", msg.type);
        switch (msg.type) {
            case COUNT:
                count_words_and_respond(&msg);
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