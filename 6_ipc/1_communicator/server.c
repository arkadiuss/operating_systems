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

void handle_msg_by_type(msg msg) {
    printf("Received message %s\n", msg.data);
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

