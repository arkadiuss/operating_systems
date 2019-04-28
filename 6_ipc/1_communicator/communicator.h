//
// Created by arkadius on 27.04.19.
//

#ifndef COMMUNICATOR_COMMUNICATOR_H
#define COMMUNICATOR_COMMUNICATOR_H

#define MSG_SIZE 30
#define MAX_CLIENTS_CNT 100


const int QKEY = 233121;

typedef struct msg {
    long type;
    char data[MSG_SIZE];
} msg;


#ifdef SV // queues from System V
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
int create_queue(int key, int flag){
    return msgget(key, 0666 | (flag == 1 ? IPC_CREAT : 0));
}
int snd_msg(int qid, msg *msg) {
    return msgsnd(qid, msg, MSG_SIZE, 0);
}
int rcv_msg(int qid, msg *msg, int type) {
    return (int) msgrcv(qid, msg, MSG_SIZE, type, 0);
}
#endif
#ifdef PX // queues from POSIX
#include <fcntl.h>
int create_queue(){

}
int snd_msg() {

}
int rcv_msg() {

}
#endif

const int TYPES_CNT = 5;
enum message_types {
    STOP = 1L, LIST = 2L, FRIENDS = 3L, INIT = 4L, ECHO = 5L
};

enum client_message_types {
    CTRL = 1L, MESSAGE = 2L
};

msg generate_init(int qid) {
    msg msg;
    sprintf(msg.data, "INIT %d", qid);
    msg.type = INIT;
    return msg;
}

typedef struct client {
    int qid;
} client;

#endif //COMMUNICATOR_COMMUNICATOR_H
