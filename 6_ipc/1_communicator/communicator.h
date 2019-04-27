//
// Created by arkadius on 27.04.19.
//

#ifndef COMMUNICATOR_COMMUNICATOR_H
#define COMMUNICATOR_COMMUNICATOR_H

const int QID = 234321;

enum message_types {
    INIT = 0L, MSG = 1L
};

typedef struct msg {
    long type;
    char data[20];
} msg;

msg generate_init(int qid) {
    msg msg;
    sprintf(msg.data, "INIT %d", qid);
    msg.type = INIT;
    return msg;
}

#endif //COMMUNICATOR_COMMUNICATOR_H
