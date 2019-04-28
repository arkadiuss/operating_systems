//
// Created by arkadius on 27.04.19.
//

#ifndef COMMUNICATOR_COMMUNICATOR_H
#define COMMUNICATOR_COMMUNICATOR_H

const int QKEY = 23312;
#define MSG_SIZE 30
#define MAX_CLIENTS_CNT 100
const int TYPES_CNT = 5;
enum message_types {
    STOP = 1L, LIST = 2L, FRIENDS = 3L, INIT = 4L, ECHO = 5L
};

enum client_message_types {
    CTRL = 1L, MESSAGE = 2L
};

typedef struct msg {
    long type;
    char data[MSG_SIZE];
} msg;

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
