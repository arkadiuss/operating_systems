//
// Created by arkadius on 30.03.19.
//

#ifndef INC_3_SENDER_CATCHER_COMMON_H
#define INC_3_SENDER_CATCHER_COMMON_H

#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

typedef enum {
    KILL = 0, QUEUE = 1, RT = 2
} Mode;

pid_t get_pid(char *pid);
int get_signal_number(char *number);
Mode get_mode(char *mode);
void set_def_sigaction(struct sigaction* act);
void send_by_mode(pid_t pid, int sig, Mode mode, union sigval data);

#endif //INC_3_SENDER_CATCHER_COMMON_H
