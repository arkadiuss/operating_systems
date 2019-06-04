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
#define MAX_PATH_LENGTH 108
int port;
char socket_path[MAX_PATH_LENGTH];
int local_socket, remote_socket;
int poll_sock;
client clients[MAX_CLIENTS];
int ccount = 0;
pthread_t connections_thread, ping_thread;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
#define SAFE_CLIENTS(block) pthread_mutex_lock(&client_mutex); block; pthread_mutex_unlock(&client_mutex);


void init_local_socket() {
    if((local_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        show_error_and_exit("Unable to create socket", 1);
    }
    struct sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, socket_path);
    if(bind(local_socket, (struct sockaddr*) &address, sizeof(address)) < 0) {
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
    if(bind(remote_socket, (struct sockaddr*) &address, sizeof(address)) < 0) {
        show_error_and_exit("Unable to bind to remote socket", 1);
    }
    if(listen(remote_socket, 12) < 0) {
        show_error_and_exit("Couldn't listen local socket", 1);
    }
}

void accept_new_client(int fd) {
    int client_socket;
    if((client_socket = accept(fd, NULL, NULL)) < 0) {
        show_error_and_exit("Unable to accept client", 1);
    }
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLRDHUP;
    event.data.fd = client_socket;
    if(epoll_ctl(poll_sock, EPOLL_CTL_ADD, client_socket, &event) < 0) {
        fprintf(stderr, "Unable to add client to poll\n");
        return;
    }
}

int get_client_by_fd(int fd) {
    for(int i = 0; i<ccount; i++) {
        if(!clients[i].disconnected && clients[i].fd == fd) return i;
    }
    return -1;
}

void handle_register_message(int fd) {
    uint16_t size;
    READ_OR_RETURN(fd, &size, MSG_SIZE_SIZE)
    char name[NAME_SIZE];
    READ_OR_RETURN(fd, name, size)
    name[size] = '\0';
    int id = ccount++;
    clients[id].fd = fd;
    strcpy(clients[id].name, name);
    clients[id].ping_requests = 0;
    clients[id].busy = 0;
    clients[id].disconnected = 0;
    printf("Client %s with socket %d connected\n", name, fd);
}

void handle_result(int fd) {
    uint16_t size;
    READ_OR_RETURN(fd, &size, MSG_SIZE_SIZE)
    char msg[MAX_FILE_SIZE];
    READ_OR_RETURN(fd, msg, size)
    int client;
    if((client = get_client_by_fd(fd)) != -1) {
        clients[client].busy--;
    }
    printf("Received message: \n %s\n", msg);
}

void handle_pong(int fd) {
    int client;
    if((client = get_client_by_fd(fd)) != -1) {
        clients[client].ping_requests--;
    }
}

void disconnect_client(int id) {
    clients[id].disconnected = 1;
    close(clients[id].fd);
    printf("Client %s disconnected\n", clients[id].name);
}

void handle_response(int fd) {
    uint8_t type;
    READ_OR_RETURN(fd, &type, TYPE_SIZE)
    SAFE_CLIENTS(
    switch(type) {
        case REGISTER:
            handle_register_message(fd);
            break;
        case RESULT:
            handle_result(fd);
            break;
        case PONG:
            handle_pong(fd);
            break;
        case UNREGISTER: {
            int id = get_client_by_fd(fd);
            if (id != -1) disconnect_client(id);
            break;
        }
    })
}

void* handle_messages(void *data){
    poll_sock = epoll_create(1);
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
            if(events[i].events & EPOLLRDHUP) {
                printf("Connection closed by the remote peer\n");
                SAFE_CLIENTS(
                int id = get_client_by_fd(events[i].data.fd);
                if(id != -1) disconnect_client(id);)
            } else if(events[i].data.fd == remote_socket || events[i].data.fd == local_socket)
                accept_new_client(events[i].data.fd);
            else
                handle_response(events[i].data.fd);
        }
    }
    return NULL;
}

void send_ping(int id) {
    uint8_t type = PING;
    clients[id].ping_requests++;
    WRITE_OR_RETURN(clients[id].fd, &type, TYPE_SIZE)
}

void* ping_clients(void* data) {
    while(1) {
        SAFE_CLIENTS(
        for (int i = 0; i < ccount; i++) {
            if (!clients[i].disconnected) {
                if (clients[i].ping_requests > 0) {
                    disconnect_client(i);
                } else {
                    send_ping(i);
                }
            }
        })
        sleep(5);
    }
}

int choose_client() {
    int min_busy = 100;
    int min_busy_i = -1;
    for(int i = 0; i<ccount; i++) {
        if(!clients[i].disconnected) {
            if (clients[i].busy == 0) return i;
            else {
                if (clients[i].busy < min_busy) {
                    min_busy = clients[i].busy;
                    min_busy_i = i;
                }
            }
        }
    }
    return min_busy_i;
}

void delegate_request(char *path) {
    char content[MAX_FILE_SIZE];
    long file_size = read_whole_file(path, content);
    if(file_size < 0) {
        return;
    }
    int client_id;
    if((client_id = choose_client()) == -1){
        fprintf(stderr, "No client to process request\n");
        return;
    }
    clients[client_id].busy++;
    uint8_t type = COUNT;
    uint16_t size = (uint16_t) file_size;
    WRITE_OR_RETURN(clients[client_id].fd, &type, TYPE_SIZE)
    WRITE_OR_RETURN(clients[client_id].fd, &size, MSG_SIZE_SIZE)
    WRITE_OR_RETURN(clients[client_id].fd, content, file_size)
}

void accept_requests(){
    while(1) {
        char filename[MAX_PATH_LENGTH];
        scanf("%s", filename);
        SAFE_CLIENTS(delegate_request(filename));
    }
}

void int_handler(int sig) {
    pthread_cancel(connections_thread);
    pthread_cancel(ping_thread);
    close(poll_sock);
    SAFE_CLIENTS(
            for(int i =0; i< ccount; i++) {
                if(!clients[i].disconnected) {
                    disconnect_client(i);
                }
            }
            )
    close(remote_socket);
    close(local_socket);
    unlink(socket_path);
    exit(0);
}

int main(int argc, char **argv) {
    validate_argc(argc, 2);
    port = as_integer(argv[1]);
    strcpy(socket_path, argv[2]);
    init_remote_socket();
    init_local_socket();

    pthread_create(&connections_thread, NULL, handle_messages, NULL);
    pthread_create(&ping_thread, NULL, ping_clients, NULL);


    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = int_handler;
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    accept_requests();
    return 0;
}