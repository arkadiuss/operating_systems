#include <stdio.h>
#include <unistd.h>
#include "communicator.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys-ops-commons.h>

int generate_number(){
    pid_t pid = getpid();
    return 123*pid + 1000;
}

int main() {
    if(msgget(QID, 0) < 0){
        show_error_and_exit("Unable to create queue", 1);
    }

    int client_qid = generate_number();
    msgget(client_qid, IPC_CREAT);

    msg init_msg = generate_init(client_qid);
    if(msgsnd(QID, &init_msg, sizeof(init_msg), 0)){

    }

    return 0;
}