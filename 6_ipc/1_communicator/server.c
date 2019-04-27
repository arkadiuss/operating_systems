#include <stdio.h>
#include "communicator.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys-ops-commons.h>

int main() {
    if(msgget(QID, IPC_CREAT) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }

    while(1) {
        if(msgrcv(QID, ))
    }
}