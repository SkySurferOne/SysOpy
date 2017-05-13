#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <string.h>
#include <bits/time.h>
#include <time.h>
#include <signal.h>
#include <wait.h>

// constants
#define SEMG_KEY 1
#define BARBER_SEM_NUM 0
#define FIFO_SEM_NUM 1
#define CUT_SEM_NUM 2

// queue api
void bqueue_init(pid_t *, int);
int bqueue_empty(pid_t *);
int bqueue_full(pid_t *);
int bqueue_put(pid_t *, pid_t);
pid_t bqueue_get(pid_t *);
void bqueue_occupy_chair(pid_t *, pid_t);
pid_t bqueue_get_customer_from_chair(pid_t *);
void bqueue_show(pid_t *);

// semaphores
void give_semaphore(int, unsigned short);
void take_semaphore(int, unsigned short);

// utils
__time_t get_time();

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif //ZAD1_COMMON_H
