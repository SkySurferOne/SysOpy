//
// Created by damian on 21.03.17.
//

#ifndef ZAD2_UTILS_H
#define ZAD2_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

struct Stack {
    struct Stack * prev;
    void * value;
};
typedef struct Stack Stack;

Stack * stackInit();
void push(Stack **, void * value);
void * pop(Stack **);
int empty(Stack *);

char * str_concat(int, ...);

#endif //ZAD2_UTILS_H
