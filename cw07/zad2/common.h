#ifndef ZAD2_COMMON_H
#define ZAD2_COMMON_H

#include <sys/stat.h>
#include <semaphore.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <wait.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>

// constants
#define BARBER_SEM_NUM 0
#define FIFO_SEM_NUM 1
#define CUT_SEM_NUM 2

#define MEMORY_NAME "memory"
#define BARBER_NAME "barber"
#define FIFO_NAME "fifo"
#define CUT_NAME "cut"

// queue api
void bqueue_init(pid_t *, int);
int bqueue_empty(pid_t *);
int bqueue_full(pid_t *);
int bqueue_put(pid_t *, pid_t);
pid_t bqueue_get(pid_t *);
void bqueue_occupy_chair(pid_t *, pid_t);
pid_t bqueue_get_customer_from_chair(pid_t *);
void bqueue_show(pid_t *);

// utils
__time_t get_time();

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif //ZAD2_COMMON_H
