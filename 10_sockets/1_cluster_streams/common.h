//
// Created by arkadius on 31.05.19.
//

#ifndef TEMPLATE_COMMON_H
#define TEMPLATE_COMMON_H

#include <string.h>
#include "sys-ops-commons.h"
#include <string.h>
#include <errno.h>

#define MAX_CLIENTS 12
#define NAME_SIZE 30
#define TYPE_SIZE 1
#define MSG_SIZE_SIZE 2
#define MAX_FILE_SIZE 1000000

#define READ_OR_RETURN(sock, buf, size) if(read(sock, buf, size) != size) { fprintf(stderr, "Unable to read message\n Error: %s \n", strerror(errno)); return; }
#define WRITE_OR_RETURN(sock, buf, size) if(write(sock, buf, size) != size) { fprintf(stderr, "Unable to write message \n"); return; }
#define WRITE_OR_RETURN_USIZE(sock, buf, size) if(read(sock, buf, size) <= 0) { fprintf(stderr, "Unable to read type \n"); return; }
typedef enum {
    LOCAL = 1, REMOTE = 2
} client_type;

typedef enum {
    REGISTER, UNREGISTER, COUNT, RESULT, PING, PONG
} message_type;

client_type get_type(const char *str) {
    if(strcmp(str, "LOCAL") == 0) return LOCAL;
    if(strcmp(str, "REMOTE") == 0) return REMOTE;
    show_error_and_exit("Type should be LOCAL or REMOTE", 1);
}

typedef struct client {
    char name[NAME_SIZE];
    int fd;
    int busy;
    int ping_requests;
    int disconnected;
} client;

#endif //TEMPLATE_COMMON_H
