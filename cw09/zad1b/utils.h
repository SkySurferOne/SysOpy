//
// Created by damian on 21.05.17.
//

#ifndef ZAD1_UTILS_H
#define ZAD1_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <bits/time.h>
#include <time.h>

#define LIST_ERR -1
#define LIST_OK 0

typedef struct fixed_array_list {
    int *data;
    int size;
    int items_num;
} fixed_array_list;

// array list
fixed_array_list *fal_init(int);
int fal_empty(fixed_array_list *);
int fal_is_full(fixed_array_list *);
int fal_add(fixed_array_list *, int);
int fal_pop(fixed_array_list *);
int fal_remove(fixed_array_list *, int);
void fal_destroy(fixed_array_list *);
int fal_len(fixed_array_list *);

// time
__time_t get_time();
__time_t get_ntime();

// colors
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif //ZAD1_UTILS_H
