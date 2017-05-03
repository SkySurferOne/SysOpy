#ifndef ZAD2_COMMON_H
#define ZAD2_COMMON_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

void printt(char*);

#define SERVER_NAME "/server"

#define MQ_FLAGS 0
#define MQ_MAXMSG 10
#define MQ_MSGSIZE 128
#define MQ_CURMSG 10
#define CLIENTS_MAX_NUM 20

#define ECHO_REQ 5
#define UPPERCASE_REQ 6
#define TIME_REQ 7
#define TERMINATE_REQ 8
#define REGISTER_REQ 9
#define LOGOUT_REQ 10

#define UNREGISTERED -2
#define REGISTER_OK 11
#define REGISTER_FAIL 13
#define PRINT 12

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void printt(char * buf) {
    printf("str: %s\n", buf);
    for (int i=0; buf[i] != '\0'; i++) {
        printf("%d ", (int) buf[i]);
    }
    printf("\n\n");
}

#endif //ZAD2_COMMON_H
