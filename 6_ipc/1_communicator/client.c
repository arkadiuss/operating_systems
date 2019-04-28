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
int msqid;
int client_qid;

int generate_number(){
    pid_t pid = getpid();
    return 123*pid + 1000;
}

int init_queues(){
    if((msqid = msgget(QKEY, 0666)) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }
    printf("Client qid %d\n", msqid);

    int client_key = generate_number();
    if((client_qid = msgget(client_key, 0666 | IPC_CREAT)) < 0){
        show_error_and_exit("Unable to create children queue", 1);
    }

    msg init_msg = generate_init(client_qid);
    if(msgsnd(msqid, &init_msg, MSG_SIZE, 0) < 0){
        show_error_and_exit("Unable to send message", 1);
    }

    msg received_message;
    if(msgrcv(client_qid, &received_message, MSG_SIZE, CTRL, 0) < 0){
        show_error_and_exit("Unable to receive id", 1);
    }
    //TODO: validation
    client_id = atoi(received_message.data);
    printf("Connected to server! id: %d", client_id);

}

void send_echo(const char *str){
    msg msg;
    msg.type = ECHO;
    strcpy(msg.data, str);
    if(msgsnd(msqid, &msg, MSG_SIZE, 0) < 0){
        fprintf(stderr, "Unable to send echo message\n");
        return;
    }
    if(msgrcv(client_qid, &msg, MSG_SIZE, CTRL, 0) < 0){
        fprintf(stderr, "Unable to receive echo message\n");
        return;
    }
    printf("Server response: %s", msg.data);
}

void handle_commands(){
    char command[MSG_SIZE];
    while(1) {
        scanf("%s", command);
        char* args[3];
        split_to_arr(args, command);
        if(strstr(args[0], "ECHO")){
            send_echo(args[1]);
        } else if(strstr(args[0], "LIST")){
            //msg.type = LIST;
        } else if (strstr(args[0], "FRIENDS")) {
            //msg.type = FRIENDS;
        }

    }
}

void listen_to_messages(){
    msg received_msg;
    while(1){
        if(msgrcv(client_qid, &received_msg, MSG_SIZE, MESSAGE, 0) < 0){
            fprintf(stderr, "Unable to read message\n");
        }
        printf("I've received a message! MESSAGE: %s\n", received_msg.data);
    }
}

int main() {
    init_queues(&msqid, &client_qid);
    if(fork() == 0){
        listen_to_messages(msqid, client_qid);
    } else {
        handle_commands(msqid, client_qid);
    }

    return 0;
}