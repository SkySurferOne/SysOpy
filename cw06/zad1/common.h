//
// Created by damian on 23.04.17.
//

#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define MSGSZ     128

#define SERVER_KEY 1

#define ECHO_REQ 5
#define UPPERCASE_REQ 6
#define TIME_REQ 7
#define TERMINATE_REQ 8
#define REGISTER_REQ 9
#define LOGOUT_REQ 10

#define PRINT 12
#define UNREGISTERED -2

#define REGISTER_OK 11

typedef struct message_buf {
    long    mtype;
    char    mtext[MSGSZ];
    pid_t senderPid;
} message_buf;

const size_t MSGBUF_SIZE = sizeof(message_buf) - sizeof(long);

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif //ZAD1_COMMON_H
