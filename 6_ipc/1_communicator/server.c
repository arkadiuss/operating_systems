#include <stdio.h>
#include "communicator.h"
#include <sys-ops-commons.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

client clients[MAX_CLIENTS_CNT];
int cur_client = 0;

void init_client(int client_key, int client_msg_key) {
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

void respond_to_echo(int client_id, char *str) {
    msg msg;
    msg.type = CTRL;
    strcpy(msg.data, str);
    if(snd_msg(clients[client_id].qid, &msg) < 0){
        fprintf(stderr, "Unable to send echo message\n");
        return;
    }
}

void send_message_to_one(int sender_id, int receiver_id, const char *content){
    msg msg;
    msg.type = MESSAGE;
    sprintf(msg.data, "From %d: %s", sender_id, content);
    if(snd_msg(clients[receiver_id].qid, &msg) < 0){
        fprintf(stderr, "Unable to send private message\n");
        return;
    }
}

void send_broadcast_message(int sender_id, const char *content){
    for(int i = 0; i < cur_client; i++){
        if(i != sender_id)
            send_message_to_one(sender_id, i, content);
    }
}

void stop_client(int id){
    clients[id].closed = 1;
    close_queue(clients[id].qid);
    close_queue(clients[id].msg_qid);
    printf("Client %d disconnected\n", id);
}

void handle_msg_by_type(msg msg) {
    printf("Received message %ld %s\n", msg.type, msg.data);
    char* args[3];
    int argc = split_to_arr(args, msg.data);
    //TODO: ARGS VALIDATION
    switch (msg.type){
        case INIT:
            //if(!is_integer(args[0]))
            init_client(atoi(args[1]), atoi(args[2]));
            break;
        case ECHO:
            respond_to_echo(atoi(args[0]), args[1]);
            break;
        case TO_ONE:
            send_message_to_one(atoi(args[0]), atoi(args[1]), args[2]);
            break;
        case TO_ALL:
            send_broadcast_message(atoi(args[0]), args[1]);
        case STOP:
            stop_client(atoi(args[0]));
        default:
            fprintf(stderr, "Unrecognized message type");
    }
}

int main() {
    int msqid;

    if((msqid = create_queue(QKEY, 1)) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }

    printf("qid %d \n", msqid);

    while(1) {
        msg received_msg;
        if(rcv_msg(msqid, &received_msg, -TYPES_CNT) < 0) { //-types count to specify priority of messages
            fprintf(stderr, "Error while receiving");
        }
        handle_msg_by_type(received_msg);
    }
}

