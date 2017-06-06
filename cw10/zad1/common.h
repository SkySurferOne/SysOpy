//
// Created by damian on 25.05.17.
//

#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_NAME_LEN 10

typedef struct {
    int id;
    int msg_type;
    int task;
    int a, b;
    int result;
    char name[MAX_NAME_LEN];
} message;

#define PATH_MAX    108
#define SUM 1
#define SUB 2
#define MUL 3
#define DIV 4
#define EXIT 5
#define CLIENT_MAX_NUM 10
#define MAX_EPOLL_EVENTS 32


#define PERFORM_TASK 6
#define RESULT 7
#define KILL 8
#define CONNECT 9
#define PING 10


#endif //ZAD1_COMMON_H
