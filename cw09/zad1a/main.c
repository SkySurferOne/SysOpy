#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "utils.h"
#include <semaphore.h>

#define BUFF_SIZE 10
#define READERS_NUM 5
#define WRITERS_NUM 5
#define MAX_VAL 20
#define VERBOSE parsedArgs->verbose
#define USLEEP_READERS_TIME 500000 // microseconds (1 000 000 [ms] = 1 [s] <=> 1 [ms] = 10^-6 [s])
#define USLEEP_WRITER_TIME 500000

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
fixed_array_list *tid_queue;

// semaphores
sem_t resourceAccess;
sem_t readCountAccess;
sem_t serviceQueue;

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
int is_my_turn(pthread_t);
void next_please();
void get_sem(sem_t *);
void release_sem(sem_t *);
void go_to_queue(pthread_t);


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
    get_sem(&readCountAccess);
    --readersNum;
    if (readersNum == 0) {
        // release res semaphore to writer
        sem_post(&resourceAccess);
    }
    release_sem(&readCountAccess);
}

void inc_readers() {
    get_sem(&readCountAccess);
    if (readersNum == 0) {
        // get res semaphore to lock writers util there won't be any reader
        get_sem(&resourceAccess);
    }
    ++readersNum;

    next_please();
    release_sem(&readCountAccess);
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
    pthread_t tid = pthread_self();

    while (1) {

        // wait for your turn
        go_to_queue(tid);
        while (!is_my_turn(tid)) {}

        // get semaphore
        get_sem(&resourceAccess);

        // remove head from queue
        next_please();

        // write and print
        srand(get_ntime());
        mod_num = (rand() % (BUFF_SIZE-1)) + 1;
        fixed_array_list *list = fal_init(BUFF_SIZE);
        for (int i=0; i<BUFF_SIZE; i++) {
            fal_add(list, i);
        }

        for (int i=0; i<mod_num; i++) {
            int index = rand() % fal_len(list);
            int rand_index = (int) fal_remove(list, index);
            int new_value = rand() % MAX_VAL;
            buffer[rand_index] = new_value;

            if (VERBOSE) {
                printf(ANSI_COLOR_GREEN "[%zu] >>TID %ld<< modified buffer[%d] with new value %d" ANSI_COLOR_RESET "\n",
                       get_time(), pthread_self(), rand_index, new_value);
            }
        }
        printf(ANSI_COLOR_GREEN "[%zu] writer with >>TID %ld<< modified buffer" ANSI_COLOR_RESET "\n", get_time(), pthread_self());
        fal_destroy(list);

        // release semaphore
        release_sem(&resourceAccess);

        usleep(USLEEP_WRITER_TIME);
    }

    return NULL;
}

void *reader_routine(void *args){
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    reader_args *params = (reader_args *) args;
    int divider = params->divider;
    int divisible;
    pthread_t tid = pthread_self();

    while (1) {

        // wait for your turn
        go_to_queue(tid);
        while (!is_my_turn(tid)) {}

        // increment readers and lock writers
        inc_readers();

        // read
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

        // decrement readers and unlock writers if there is no readers anymore
        dec_readers();

        usleep(USLEEP_READERS_TIME);
    }

    return NULL;
}

inline void get_sem(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        perror("sem_wait error\n");
        exit(EXIT_FAILURE);
    }
}

inline void release_sem(sem_t *sem) {
    if (sem_post(sem) == -1) {
        perror("sem_wait error\n");
        exit(EXIT_FAILURE);
    }
}

void go_to_queue(pthread_t tid) {
    get_sem(&serviceQueue);
    // printf(">>TID %ld<< is checking queue\n", tid);
    // fal_print(tid_queue);

    if (fal_add(tid_queue, tid) == LIST_ERR) {
        perror("fal_add error: queue is full\n");
    }

    release_sem(&serviceQueue);
}

int is_my_turn(pthread_t tid) {
    get_sem(&serviceQueue);

    if (fal_get(tid_queue) == tid) {
        release_sem(&serviceQueue);
        return 1;
    }
    else {
        release_sem(&serviceQueue);
        return 0;
    }
}

void next_please() {
    fal_pop(tid_queue);
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

    // init semaphores
    sem_init(&resourceAccess, 0, 1);
    sem_init(&readCountAccess, 0, 1);
    sem_init(&serviceQueue, 0, 1);

    // init queue
    tid_queue = fal_init(READERS_NUM + WRITERS_NUM);
}

void clean() {
    if (sem_destroy(&resourceAccess) < 0) {
        perror("sem_destroy error");
        exit(EXIT_FAILURE);
    }

    if (sem_destroy(&readCountAccess) < 0) {
        perror("sem_destroy error");
        exit(EXIT_FAILURE);
    }

    if (sem_destroy(&serviceQueue) < 0) {
        perror("sem_destroy error");
        exit(EXIT_FAILURE);
    }

    fal_destroy(tid_queue);
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
