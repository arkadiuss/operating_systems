#include <stdio.h>
#include "communicator.h"
#include <sys-ops-commons.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

client clients[MAX_CLIENTS_CNT];
int cur_client = 0;
int msqid;

void init_client(int pid, int client_key, int client_msg_key) {
    printf("Initializing client with id %d, key %d\n", cur_client, client_key);
    int qid, msg_qid;
    if((qid = create_queue(client_key, 0)) < 0){
        fprintf(stderr, "Unable to open child queue");
        return;
    }
    if((msg_qid = create_queue(client_msg_key, 0)) < 0){
        fprintf(stderr, "Unable to open child queue");
        return;
    }
    clients[cur_client].pid = pid;
    clients[cur_client].qid = qid;
    clients[cur_client].msg_qid = msg_qid;
    clients[cur_client].closed = 0;
    msg msg;
    msg.type = CTRL;
    sprintf(msg.data, "%d", cur_client);
    if(snd_msg(qid, &msg) < 0){
        fprintf(stderr, "Unable to send initializing message: %s", strerror(errno));
        return;
    }
    cur_client++;
    printf("Client %d initialized successfully, qid: %d\n", cur_client-1, qid);
}

void send_message(int id, msg *msg){
    if(snd_msg(clients[id].qid, msg) < 0){
        fprintf(stderr, "Unable to send message. Error: %s\n ", strerror(errno));
        return;
    }
    kill(clients[id].pid, SIGRTMIN);
}

void respond_to_echo(int client_id, char *str) {
    msg msg;
    msg.type = CTRL;
    strcpy(msg.data, str);
    send_message(client_id, &msg);
}

void send_list(int client_id) {
    msg msg;
    msg.type = CTRL;
    strcat(msg.data, "Current users:\n");
    char line[MSG_SIZE];
    for(int i = 0; i < cur_client; i++){
        if(!clients[i].closed && client_id != i){
            sprintf(line, "Client %d", i);
            if(clients[client_id].friends[i]) {
                strcat(line, " friend\n");
            } else {
                strcat(line, "\n");
            }
            strcat(msg.data, line);
        }
    }
    send_message(client_id, &msg);
}

void send_message_to_one(int sender_id, int receiver_id, const char *content){
    if(receiver_id >= 0 && receiver_id < cur_client && !clients[receiver_id].closed) {
        msg msg;
        msg.type = MESSAGE;
        sprintf(msg.data, "From %d: %s", sender_id, content);
        send_message(receiver_id, &msg);
    }
}

void send_message_to_friends(int sender_id, const char *content){
    for(int i = 0; i < cur_client; i++){
        if(i != sender_id && clients[sender_id].friends[i])
            send_message_to_one(sender_id, i, content);
    }
}

void send_broadcast_message(int sender_id, const char *content){
    for(int i = 0; i < cur_client; i++){
        if(i != sender_id)
            send_message_to_one(sender_id, i, content);
    }
}

void disconnect_client(int id){
    if(!clients[id].closed){
        clients[id].closed = 1;
        printf("Client %d disconnected\n", id);
    }
}

void stop_all_clients() {
    msg msg;
    msg.type = STOP_CLIENT;
    for(int i = 0; i < cur_client; i++){
        send_message(i, &msg);
        disconnect_client(i);
    }
}

void set_friends(int n, char** friends){
    int req_id = atoi(friends[0]);
    for(int i = 0; i < cur_client; i++){
        clients[req_id].friends[i] = 0;
    }
    for(int i = 1; i < n; i++){
        int fid = atoi(friends[i]);
        if(fid >= 0 && fid < cur_client)
            clients[req_id].friends[fid] = 1;
    }
}

// IT REALLY HURTS WHEN YOU DO O(n^2) ALGORITHM WHEN O(nlogn) IS COMMONLY KNOWN :(
void add_friends(int n, char** friends){
    int req_id = atoi(friends[0]);
    for(int i = 1; i < n; i++){
        int fid = atoi(friends[i]);
        if(fid >= 0 && fid < cur_client)
            clients[req_id].friends[fid] = 1;
    }
}

// IT REALLY HURTS WHEN YOU DO O(n^2) ALGORITHM WHEN O(nlogn) IS COMMONLY KNOWN :(
void remove_friends(int n, char** friends){
    int req_id = atoi(friends[0]);
    for(int i = 1; i < n; i++){
        int fid = atoi(friends[i]);
        if(fid >= 0 && fid < cur_client)
            clients[req_id].friends[fid] = 0;
    }
}

void handle_msg_by_type(msg msg) {
    printf("Received message %ld %s\n", msg.type, msg.data);

    char* args[20];
    int argc = split_to_arr(args, msg.data);
    //TODO: ARGS VALIDATION
    switch (msg.type){
        case INIT:
            //if(!is_integer(args[0]))
            init_client(atoi(args[1]), atoi(args[2]), atoi(args[3]));
            break;
        case ECHO:
            respond_to_echo(atoi(args[0]), args[1]);
            break;
        case LIST:
            send_list(atoi(args[0]));
            break;
        case FRIENDS:
            set_friends(argc, args);
            break;
        case ADD:
            add_friends(argc, args);
            break;
        case REMOVE:
            remove_friends(argc, args);
            break;
        case TO_ONE:
            send_message_to_one(atoi(args[0]), atoi(args[1]), args[2]);
            break;
        case TO_ALL:
            send_broadcast_message(atoi(args[0]), args[1]);
            break;
        case TO_FRIENDS:
            send_message_to_friends(atoi(args[0]), args[1]);
            break;
        case STOP:
            disconnect_client(atoi(args[0]));
            break;
        default:
            fprintf(stderr, "Unrecognized message type");
    }
}

void int_handler(int signum) {
    stop_all_clients();
    close_queue(msqid);
    exit(0);
}

void handle_signals() {
    struct sigaction int_act;

    int_act.sa_handler = int_handler;
    sigemptyset (&int_act.sa_mask);
    sigaction(SIGINT, &int_act, NULL);
}

int main() {

    if((msqid = create_queue(QKEY, 1)) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }

    printf("Server qid %d \n", msqid);

    handle_signals();

    while(1) {
        msg received_msg;
        if(rcv_msg(msqid, &received_msg, -TYPES_CNT) < 0) { //-types count to specify priority of messages
            fprintf(stderr, "Error while receiving\n");
        }
        handle_msg_by_type(received_msg);
    }
}

