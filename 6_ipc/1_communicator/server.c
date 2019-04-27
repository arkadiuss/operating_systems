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
    msg.type = 1;
    sprintf(msg.data, "%d", cur_client);
    if(msgsnd(qid, &msg, MSG_SIZE, 0) < 0){
        fprintf(stderr, "Unable to send initializing message: %s", strerror(errno));
        return;
    }
    cur_client++;
    printf("Client %d initialized successfully\n", cur_client-1);
}

void handle_msg_by_type(msg msg) {
    printf("Received message %s\n", msg.data);
    char* args[3];
    int argc = split_to_arr(args, msg.data);
    //TODO: ARGS VALIDATION
    switch (msg.type){
        case INIT:
            //if(!is_integer(args[0]))
            init_client(atoi(args[0]));
            break;
        case LIST:

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
        if(msgrcv(msqid, &received_msg, MSG_SIZE, 0, 0) < 0) { //-types count to specify priority of messages
            fprintf(stderr, "Error while receiving");
        }
        handle_msg_by_type(received_msg);
    }
}

