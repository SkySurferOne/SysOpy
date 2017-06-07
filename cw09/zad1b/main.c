#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "utils.h"

#define BUFF_SIZE 10
#define READERS_NUM 5
#define WRITERS_NUM 5
#define MAX_VAL 20
#define VERBOSE parsedArgs->verbose
#define USLEEP_READERS_TIME 2000000 // microseconds (1 000 000 [ms] = 1 [s] <=> 1 [ms] = 10^-6 [s])
#define USLEEP_WRITER_TIME 1000000

// structs
typedef struct ParsedArgs {
    char *exePath;
    int verbose;
} ParsedArgs;

typedef struct reader_args {
    int divider;
} reader_args;

// globals
ParsedArgs *parsedArgs;
pthread_t * readers;
pthread_t * writers;
int * buffer;
int readersNum = 0;
pthread_mutex_t inc_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dec_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;

// api
void parse_args(int, char **);
void *writer_routine(void *);
void *reader_routine(void *);
void init();
void clean();
void make_threads();
void sig_handler(int);
void dec_readers();
void inc_readers();
void parse_args(int, char **);

int main(int argc, char **argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parsedArgs->verbose = 0;
    parse_args(argc, argv);

    init();
    make_threads();
    clean();

    return 0;
}

void dec_readers() {
    pthread_mutex_lock(&inc_mutex);
    --readersNum;
    if (readersNum == 0)
        pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&inc_mutex);
}

void inc_readers() {
    pthread_mutex_lock(&inc_mutex);
    ++readersNum;
    pthread_mutex_unlock(&inc_mutex);
}

void sig_handler(int sig) {
    printf(ANSI_COLOR_YELLOW "Closing signal caught. Closing the program..." ANSI_COLOR_RESET "\n");

    for (int i=0; i<READERS_NUM; i++) {
        if (pthread_cancel(readers[i]) == -1) {
            perror("canceling thread error");
            exit(EXIT_FAILURE);
        }
    }

    for (int i=0; i<WRITERS_NUM; i++) {
        if (pthread_cancel(writers[i]) == -1) {
            perror("canceling thread error");
            exit(EXIT_FAILURE);
        }
    }
}

void *writer_routine(void *args) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int mod_num;

    while (1) {
        usleep(USLEEP_WRITER_TIME);

        pthread_mutex_lock(&inc_mutex);
        while (readersNum != 0) {
            printf("Writer checking %d\n", readersNum);
            pthread_cond_wait(&cond, &inc_mutex);
        }

        // write and print
        srand(get_ntime());
        mod_num = (rand() % (BUFF_SIZE-1)) + 1;
        fixed_array_list *list = fal_init(BUFF_SIZE);
        for (int i=0; i<BUFF_SIZE; i++) {
            fal_add(list, i);
        }

        for (int i=0; i<mod_num; i++) {
            int index = rand() % fal_len(list);
            int rand_index = fal_remove(list, index);
            int new_value = rand() % MAX_VAL;
            buffer[rand_index] = new_value;

            if (VERBOSE) {
                printf(ANSI_COLOR_GREEN "[%zu] >>TID %ld<< modified buffer[%d] with new value %d" ANSI_COLOR_RESET "\n",
                       get_time(), pthread_self(), rand_index, new_value);
            }
        }
        printf(ANSI_COLOR_GREEN "[%zu] writer with >>TID %ld<< modified buffer" ANSI_COLOR_RESET "\n", get_time(), pthread_self());
        fal_destroy(list);

        pthread_mutex_unlock(&inc_mutex);

    }

    return NULL;
}

void *reader_routine(void *args){
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    reader_args *params = (reader_args *) args;
    int divider = params->divider;
    int divisible;

    while (1) {
        usleep(USLEEP_READERS_TIME);

        inc_readers();

        divisible = 0;
        for (int i=0; i<BUFF_SIZE; i++) {
            int num = buffer[i];
            if (num % divider == 0) {
                ++divisible;
                if (VERBOSE) {
                    printf(ANSI_COLOR_CYAN "[%zu] >>TID %ld<< found number divisible by %d, buffer[%d] == %d"
                                   ANSI_COLOR_RESET "\n",
                           get_time(), pthread_self(), divider, i, num);
                }
            }
        }
        printf(ANSI_COLOR_CYAN "[%zu] reader with >>TID %ld<< found %d numbers divisible by %d"
                       ANSI_COLOR_RESET "\n",
               get_time(), pthread_self(), divisible, divider);

        dec_readers();
    }

    return NULL;
}

void init() {
    buffer = (int *) calloc(BUFF_SIZE, sizeof(int));
    srand(get_ntime());
    for (int i=0; i<BUFF_SIZE; i++) {
        buffer[i] = rand() % MAX_VAL;
    }

    readers = (pthread_t *) calloc((size_t) READERS_NUM, sizeof(pthread_t));
    writers = (pthread_t *) calloc((size_t) WRITERS_NUM, sizeof(pthread_t));

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
}

void clean() {
    if (pthread_mutex_destroy(&inc_mutex) < 0) {
        perror("pthread_mutex_destroy error");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_destroy(&dec_mutex) < 0) {
        perror("pthread_mutex_destroy error");
        exit(EXIT_FAILURE);
    }

    free(buffer);
    free(parsedArgs);
}

void make_threads() {
    srand(get_ntime());

    for (int i=0; i<READERS_NUM; i++) {
        int divider = (rand() % (MAX_VAL-1)) + 1;
        reader_args *params = (reader_args *) calloc(1, sizeof(reader_args));
        params->divider = divider;

        if (pthread_create(&readers[i], NULL, reader_routine, (void *) params) == -1) {
            perror("creating thread error");
            exit(EXIT_FAILURE);
        }
    }

    for (int i=0; i<WRITERS_NUM; i++) {
        if (pthread_create(&writers[i], NULL, writer_routine, NULL) == -1) {
            perror("creating thread error");
            exit(EXIT_FAILURE);
        }
    }

    for (int i=0; i<READERS_NUM; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i=0; i<WRITERS_NUM; i++) {
        pthread_join(writers[i], NULL);
    }
}

void parse_args(int argc, char **argv) {
    parsedArgs->exePath = argv[0];
    if (argc >= 2) {
        if (strstr(argv[1], "-i") != NULL) {
            parsedArgs->verbose = 1;
        }
        else {
            printf("There is no such option. Write '-i' to turn on verbose mode or omit it.\n");
            exit(EXIT_FAILURE);
        }
    }
}
