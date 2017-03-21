//
// Created by damian on 21.03.17.
//
#include "utils.h"


Stack * stackInit() {
    return NULL;
}

void push(Stack ** stack, void * value) {
    if (*stack == NULL) {
        *stack = (Stack *) malloc(sizeof(Stack));
        (*stack)->prev = NULL;
        (*stack)->value = value;
    } else {
        Stack * newItem = (Stack *) malloc(sizeof(Stack));
        newItem->value = value;
        newItem->prev = (*stack);
        *stack = newItem;
    }
}

void * pop(Stack ** stack) {
    if (*stack == NULL) return NULL;
    Stack * tmp = (*stack);
    void * item = tmp->value;
    (*stack) = (*stack)->prev;
    free(tmp);
    return item;
}

int empty(Stack * stack) {
    return (stack == NULL);
}

char * str_concat(int num, ...) {
    int size = 0;
    char *buf;
    char * args[num];
    va_list valist;
    va_start(valist, num);

    for (int i=0; i<num; i++) {
        args[i] = va_arg(valist, char *);
        size += strlen(args[i]);
    }

    if (size <= 0) return "";
    buf = (char *) malloc(sizeof(char) * size + 1);

    strcpy(buf, args[0]);
    for (int i=1; i<num; i++) {
        strcat(buf, args[i]);
    }
    buf[size] = '\0';
    return buf;
}

