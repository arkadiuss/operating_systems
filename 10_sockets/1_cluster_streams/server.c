#include <stdio.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>

#define MAX_EVENTS 100
int port;
char socket_path[108];
int local_socket, remote_socket;
client clients[MAX_CLIENTS];
int ccount = 0;

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
    if(listen(local_socket, MAX_CLIENTS) < 0) {
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
    int poll_sock = epoll_create(1);
    struct epoll_event event_local, event_remote, *events;
    events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
    event_local.events = EPOLLIN;
    event_local.data.fd = local_socket;
    if(epoll_ctl(poll_sock, EPOLL_CTL_ADD, local_socket, &event_local) < 0){
        show_error_and_exit("Unable to observe local socket", 1);
    }
    event_remote.events = EPOLLIN;
    event_remote.data.fd = remote_socket;
    if(epoll_ctl(poll_sock, EPOLL_CTL_ADD, remote_socket, &event_remote) < 0){
        show_error_and_exit("Unable to observe remote socket", 1);
    }
    while(1) {
        int size;
        if((size = epoll_wait(poll_sock, events, MAX_EVENTS, -1)) < 0) {
            show_error_and_exit("Unable to wait for connections", 1);
        }
        for(int i = 0; i < size; i++) {
            int client_socket;
            if((client_socket = accept(events[i].data.fd, NULL, NULL)) < 0) {
                show_error_and_exit("Unable to accept client", 1);
            }
            printf("got client before name!!!\n");
            char name[NAME_SIZE];
            if(read(client_socket, name, NAME_SIZE) < 0) {
                fprintf(stderr, "Unable to receive register message");
                continue;
            }
            int id = ccount++;
            clients[id].fd = client_socket;
            strcpy(clients[id].name, name);
            printf("got client %s!!!\n", name);
        }
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