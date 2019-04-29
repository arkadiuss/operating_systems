#include <stdio.h>
#include <unistd.h>
#include "communicator.h"
#include <sys-ops-commons.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int client_id;
int msqid;
int client_qid;
int client_msg_qid;

int generate_number(){
    pid_t pid = getpid();
    return 123*pid + 1000;
}

msg generate_init(int client_key, int client_msg_key) {
    msg msg;
    sprintf(msg.data, "INIT %d %d %d", getpid(), client_key, client_msg_key);
    msg.type = INIT;
    return msg;
}

int init_queues(){
    if((msqid = create_queue(QKEY, 0)) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }
    printf("Client qid %d\n", msqid);

    int client_key = generate_number();
    if((client_qid = create_queue(client_key, 1)) < 0){
        show_error_and_exit("Unable to create children queue", 1);
    }

    int client_msg_key = client_key + 1;
    if((client_msg_qid = create_queue(client_msg_key, 1)) < 0){
        show_error_and_exit("Unable to create children message queue", 1);
    }

    msg init_msg = generate_init(client_key, client_msg_key);
    if(snd_msg(msqid, &init_msg) < 0){
        show_error_and_exit("Unable to send message", 1);
    }

    msg received_message;
    if(rcv_msg(client_qid, &received_message, CTRL) < 0){
        show_error_and_exit("Unable to receive id", 1);
    }
    //TODO: validation
    client_id = atoi(received_message.data);
    printf("Connected to server! id: %d\n", client_id);

}

void send_echo(const char *str){
    msg msg;
    msg.type = ECHO;
    sprintf(msg.data, "%d %s", client_id, str);
    if(snd_msg(msqid, &msg) < 0){
        fprintf(stderr, "Unable to send echo message\n");
        return;
    }
}

void send_message(int id, const char *str){
    msg msg;
    msg.type = TO_ONE;
    sprintf(msg.data, "%d %d %s", client_id, id, str);
    if(snd_msg(msqid, &msg) < 0){
        fprintf(stderr, "Unable to send private message\n");
        return;
    }
}

void send_broadcast_message(const char *str){
    msg msg;
    msg.type = TO_ALL;
    sprintf(msg.data, "%d %s", client_id, str);
    if(snd_msg(msqid, &msg) < 0){
        fprintf(stderr, "Unable to send braodcast message\n");
        return;
    }
}

void close_queues(){
    close_queue(client_qid);
    close_queue(client_msg_qid);
}

void stop_client(){
    msg msg;
    msg.type = STOP;
    sprintf(msg.data, "%d", client_id);
    if(snd_msg(msqid, &msg) < 0){
        fprintf(stderr, "Unable to send stop message\n");
        return;
    }
    close_queues();
}

void handle_commands(){
    char command[MSG_SIZE];
    char str[MSG_SIZE];
    while(1) {
        scanf("%s", command);
        if(strstr(command, "ECHO")){
            scanf("%s", str);
            send_echo(str);
        } else if(strstr(command, "2ONE")){
            int id;
            scanf("%d", &id);
            scanf("%s", str);
            send_message(id, str);
        } else if(strstr(command, "2ALL")){
            scanf("%s", str);
            send_broadcast_message(str);
        } else if (strstr(command, "STOP")) {
            stop_client();
            break;
        }

    }
}

void stopped_by_server(){
    printf("Stopping message from server. Exiting... \n");
    close_queues();
    exit(0);
}

void listen_to_message(){
    msg received_msg;
    if(rcv_msg(client_qid, &received_msg, 0) < 0){
        fprintf(stderr, "Unable to read message\n");
    }
    printf("%d %d revdf\n", received_msg.type, STOP_CLIENT);
    if(received_msg.type == STOP_CLIENT){
        stopped_by_server();
    } else {
        printf("I've received a message! MESSAGE: %s\n", received_msg.data);
    }
}

void int_handler(int signum) {
    stop_client();
    exit(0);
}

void handle_signals(){
    struct sigaction lst_act, int_act;

    lst_act.sa_handler = listen_to_message;
    sigemptyset (&lst_act.sa_mask);
    sigaction(SIGRTMIN, &lst_act, NULL);

    int_act.sa_handler = int_handler;
    sigemptyset (&int_act.sa_mask);
    sigaction(SIGINT, &int_act, NULL);
}

int main() {
    init_queues();
    handle_signals();
    handle_commands();
    return 0;
}