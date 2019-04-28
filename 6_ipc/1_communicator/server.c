#include <stdio.h>
#include "communicator.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys-ops-commons.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

client clients[MAX_CLIENTS_CNT];
int cur_client = 0;

void init_client(int qid) {
    printf("Initializing client with id %d, qid %d\n", cur_client, qid);
    clients[cur_client].qid = qid;
    msg msg;
    msg.type = CTRL;
    sprintf(msg.data, "%d", cur_client);
    if(msgsnd(qid, &msg, MSG_SIZE, 0) < 0){
        fprintf(stderr, "Unable to send initializing message: %s", strerror(errno));
        return;
    }
    cur_client++;
    printf("Client %d initialized successfully\n", cur_client-1);
}

void respond_to_echo(int client_id, char *str) {
    msg msg;
    msg.type = CTRL;
    strcpy(msg.data, str);
    if(msgsnd(clients[client_id].qid, &msg, MSG_SIZE, 0) < 0){
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
            init_client(atoi(args[1]));
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

    if((msqid = msgget(QKEY, 0666 | IPC_CREAT)) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }

    printf("qid %d \n", msqid);

    while(1) {
        msg received_msg;
        if(msgrcv(msqid, &received_msg, MSG_SIZE, -TYPES_CNT, 0) < 0) { //-types count to specify priority of messages
            fprintf(stderr, "Error while receiving");
        }
        handle_msg_by_type(received_msg);
    }
}

