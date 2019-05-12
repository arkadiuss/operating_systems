//
// Created by arkadius on 27.04.19.
//

#ifndef COMMUNICATOR_COMMUNICATOR_H
#define COMMUNICATOR_COMMUNICATOR_H

#define MSG_SIZE 50
#define MAX_CLIENTS_CNT 100

#include <stdlib.h>

const int QKEY = 2331231;

typedef struct msg {
    long type;
    char data[MSG_SIZE];
} msg;


const int TYPES_CNT = 10;
enum message_types {
    STOP = 1L, LIST = 2L, FRIENDS = 3L, INIT = 4L, ECHO = 5L, TO_ONE = 6L, TO_ALL = 7L, TO_FRIENDS = 8L,
    ADD = 9L, REMOVE = 10L
};

#ifdef SV // queues from System V
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int create_queue(int key, int flag){
    key_t hkey = ftok(getenv("HOME"), key);
    return msgget(hkey, 0666 | (flag == 1 ? IPC_CREAT : 0));
}
int snd_msg(int qid, msg *msg) {
    return msgsnd(qid, msg, MSG_SIZE, 0);
}
int rcv_msg(int qid, msg *msg, int type) {
    return (int) msgrcv(qid, msg, MSG_SIZE, type, 0);
}
void close_queue(int qid, int qkey, int flag){
    if(flag)
        msgctl(qid, IPC_RMID, NULL);
}
#endif
#ifdef PX // queues from POSIX
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <string.h>


int create_queue(int key, int flag){
    char name[11];
    sprintf(name, "/q%d", key);
    struct mq_attr attr;
    memset(&attr, 0, sizeof attr);
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 9;
    return mq_open(name, O_RDWR | (flag == 1 ? O_CREAT : 0), 0664, &attr);
}
int snd_msg(int qid, msg *msg) {
    int prt = TYPES_CNT - msg->type + 1;
    return mq_send(qid, msg->data, MSG_SIZE, prt);
}
int rcv_msg(int qid, msg *msg, int type) {
    char buffer[MSG_SIZE];
    int prt;
    int res = (int) mq_receive(qid, buffer, MSG_SIZE, &prt);
    msg->type = TYPES_CNT - prt + 1;
    strcpy(msg->data, buffer);
    return res;
}
void close_queue(int qid, int qkey, int flag){
    mq_close(qid);
    if(flag){
        char name[11];
        sprintf(name, "/q%d", qkey);
        mq_unlink(name);
    }
}
#endif

enum client_message_types {
    CTRL = 1L, MESSAGE = 2L, STOP_CLIENT = 3L
};

typedef struct client {
    int qid;
    int msg_qid;
    int closed;
    int pid;
    int friends[MAX_CLIENTS_CNT];
} client;

#endif //COMMUNICATOR_COMMUNICATOR_H
