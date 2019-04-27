#include <stdio.h>
#include <unistd.h>
#include "communicator.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys-ops-commons.h>
#include <stdlib.h>
#include <string.h>

int client_id;

int generate_number(){
    pid_t pid = getpid();
    return 123*pid + 1000;
}

int main() {
    int msqid;
    if((msqid = msgget(QKEY, 0666)) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }
    printf("Client qid %d\n", msqid);

    int client_key = generate_number();
    int client_msqid;
    if((client_msqid = msgget(client_key, 0666 | IPC_CREAT)) < 0){
        show_error_and_exit("Unable to create children queue", 1);
    }

    msg init_msg = generate_init(client_msqid);
    if(msgsnd(msqid, &init_msg, MSG_SIZE, 0) < 0){
        show_error_and_exit("Unable to send message", 1);
    }

    msg received_message;
    if(msgrcv(client_msqid, &received_message, MSG_SIZE, 0, 0) < 0){
        show_error_and_exit("Unable to receive id", 1);
    }
    //TODO: validation
    client_id = atoi(received_message.data);
    printf("Connected to server! id: %d", client_id);

    char command[MSG_SIZE];
    while(1) {
        scanf("%s", command);
        char* args[3];
        split_to_arr(args, command);
        msg msg;
        if(strstr(args[0], "LIST")){
            msg.type = LIST;
        } else if (strstr(args[0], "FRIENDS")) {
            msg.type = FRIENDS;
        }
        strcpy(msg.data, command);
        msgsnd(msqid, &msg, MSG_SIZE, 0);
    }

    return 0;
}