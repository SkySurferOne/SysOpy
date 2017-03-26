//
// Created by damian on 26.03.17.
//
#include <stdlib.h>

int main() {
    const unsigned int eat = 16000000;
    unsigned int i = eat;
    char *hungry = (char *) malloc(sizeof(char)*eat);

    while(i > 0) {
        hungry[i] = 'x';
        i--;
    }
    return 0;
}