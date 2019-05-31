#include <stdio.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>

int port;
char socket_path[108];
int local_socket, remote_socket;

void init_local_socket() {
    if((local_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        show_error_and_exit("Unable to create socket", 1);
    }
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, socket_path);
    if(bind(local_socket, &address, sizeof(address)) < 0) {
        show_error_and_exit("Unable to bind to local socket", 1);
    }
    if(listen(local_socket, 12) < 0) {
        show_error_and_exit("Couldn't listen local socket", 1);
    }
}
void init_remote_socket(){
    if((remote_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        show_error_and_exit("Unable to create socket", 1);
    }
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if(bind(remote_socket, &address, sizeof(address)) < 0) {
        show_error_and_exit("Unable to bind to remote socket", 1);
    }
    if(listen(remote_socket, 12) < 0) {
        show_error_and_exit("Couldn't listen local socket", 1);
    }
}

void* accept_connections(void* data){
    while(1) {
        int client_socket;
        if((client_socket = accept(local_socket, NULL, NULL)) < 0) {
            show_error_and_exit("Unable to accept client", 1);
        }
        printf("got client!!!\n");
    }
    return NULL;
}

int main(int argc, char **argv) {
    validate_argc(argc, 2);
    port = as_integer(argv[1]);
    strcpy(socket_path, argv[2]);
    init_remote_socket();
    init_local_socket();

    pthread_t accept_connections_thread;
    pthread_create(&accept_connections_thread, NULL, accept_connections, NULL);
    pthread_join(accept_connections_thread, NULL);
    return 0;
}