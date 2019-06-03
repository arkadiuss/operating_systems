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

#define READ_OR_RETURN(sock, from, buf) unsigned int siz = (sock == local_socket ? sizeof(struct sockaddr_un) : sizeof(struct sockaddr_in)); if(recvfrom(sock, buf, sizeof(message), 0, (struct sockaddr *) from, &siz) != sizeof(message)) { fprintf(stderr, "Unable to read message\n Error: %s \n", strerror(errno)); return; }
#define SEND_OR_RETURN(sock, to, buf) unsigned int siz = (sock == local_socket ? sizeof(struct sockaddr_un) : sizeof(struct sockaddr_in)); if(sendto(sock, buf, sizeof(message), 0, to, siz) != sizeof(message)) { fprintf(stderr, "Unable to write message \n Error: %s \n", strerror(errno)); return; }


client clients[MAX_CLIENTS];
int ccount = 0;
pthread_t connections_thread, ping_thread;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
#define SAFE_CLIENTS(block) pthread_mutex_lock(&client_mutex); block; pthread_mutex_unlock(&client_mutex);


void init_local_socket() {
    if((local_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        show_error_and_exit("Unable to create socket", 1);
    }
    struct sockaddr_un address;
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, socket_path);
    if(bind(local_socket, &address, sizeof(struct sockaddr_un)) < 0) {
        show_error_and_exit("Unable to bind to local socket", 1);
    }
}
void init_remote_socket(){
    if((remote_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        show_error_and_exit("Unable to create socket", 1);
    }
    struct sockaddr_in address;
    memset(&address, 0, sizeof(struct sockaddr_in));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if(bind(remote_socket, &address, sizeof(struct sockaddr_in)) < 0) {
        show_error_and_exit("Unable to bind to remote socket", 1);
    }
}

int cmp_addr(struct sockaddr *x, struct sockaddr *y) {
    if(x->sa_family != y->sa_family) return 1;

    if (x->sa_family == AF_UNIX) {
        struct sockaddr_un *xun = (void*)x, *yun = (void*)y;
        return strcmp(xun->sun_path, yun->sun_path);
    } else if (x->sa_family == AF_INET) {
        struct sockaddr_in *xin = (void*)x, *yin = (void*)y;
        return ntohl(xin->sin_addr.s_addr) != ntohl(yin->sin_addr.s_addr);
    }
    return 1;
}

int get_client_by_addr(struct sockaddr *addr) {
    for(int i = 0; i<ccount; i++) {
        if(!clients[i].disconnected && cmp_addr(clients[i].addr, addr) == 0) return i;
    }
    return -1;
}

void handle_register_message(message *msg, struct sockaddr *addr, int socket) {
    int id = ccount++;
    clients[id].socket = socket;
    size_t size =  socket == local_socket ? sizeof(struct sockaddr_un) : sizeof(struct sockaddr_in);
    clients[id].addr = malloc(size);
    memcpy(clients[id].addr, addr, size);
    strcpy(clients[id].name, msg->msg);
    clients[id].ping_requests = 0;
    clients[id].busy = 0;
    clients[id].disconnected = 0;
    printf("Client %s connected\n", clients[id].name);
}

void handle_result(message *msg, int clientid) {
    clients[clientid].busy--;
    printf("Received message: \n %s\n", msg->msg);
}

void handle_pong(message *msg, int clientid) {
    clients[clientid].ping_requests--;
}

void disconnect_client(int id) {
    clients[id].disconnected = 1;
    printf("Client %s disconnected\n", clients[id].name);
}

void handle_response(int socket) {
    message msg;
    memset(&msg, 0 , sizeof(message));
    struct sockaddr addr;
//    READ_OR_RETURN(socket, &addr, &msg)
    socklen_t siz = sizeof(struct sockaddr_un);
    if(recvfrom(socket, &msg, sizeof(message), 0, (struct sockaddr *) &addr, &siz) != sizeof(message)) {
        fprintf(stderr, "Unable to read message\n Error: %s \n", strerror(errno)); return;
    }
    int clientid = get_client_by_addr(&addr);
    //printf("Got message %d %d %s %d %d\n ", msg.type, msg.msg_len, msg.msg, REGISTER, clientid);
    if(clientid == -1 && msg.type != REGISTER) return;
    SAFE_CLIENTS(
    switch(msg.type) {
        case REGISTER:
            handle_register_message(&msg, &addr, socket);
            break;
        case RESULT:
            handle_result(&msg, clientid);
            break;
        case PONG:
            handle_pong(&msg, clientid);
            break;
        case UNREGISTER: {
            disconnect_client(clientid);
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
//                SAFE_CLIENTS(
//                int id = get_client_by_addr(events[i].data.fd);
//                if(id != -1) disconnect_client(id);)
            } else {
                handle_response(events[i].data.fd);
            }
        }
    }
    return NULL;
}

void send_ping(int id) {
    message msg;
    msg.type = PING;
    clients[id].ping_requests++;
    SEND_OR_RETURN(clients[id].socket, clients[id].addr, &msg)
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
    message msg;
    msg.type = COUNT;
    long file_size = read_whole_file(path, msg.msg);
    msg.msg_len = file_size;
    if(file_size < 0) {
        return;
    }
    int client_id;
    if((client_id = choose_client()) == -1){
        fprintf(stderr, "No client to process request\n");
        return;
    }
    clients[client_id].busy++;
    SEND_OR_RETURN(clients[client_id].socket, clients[client_id].addr, &msg)
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