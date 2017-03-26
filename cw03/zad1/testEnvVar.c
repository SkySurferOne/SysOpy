//
// Created by damian on 26.03.17.
//
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("%s\n", "Syntax: <environment variable>");
        exit(1);
    }

    char *envVar = getenv(argv[1]);
    if (!envVar)
        printf("Env variable %s doesn't exists\n", argv[1]);
    else
        printf("%s = %s\n", argv[1], envVar);

    return 0;

}