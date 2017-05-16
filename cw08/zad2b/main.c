#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define THREADS_NUM 5
#define SUSP_TIME 2

// structs
typedef struct thead_args {
    int id;
} thread_args;

// api
void clean_after();
void init();
void make_threads();
void *action(void *);
void started_threads_incre();
void ended_threads_incre();
void print_summary();
__time_t get_time();

// globals
pthread_t *pthreads;
static int startedThreads = 0, endedThreads = 0;
pthread_mutex_t mutex;


int main(int argc, char **argv) {
    init();
    make_threads();

    print_summary();
    clean_after();
    return 0;
}

void print_summary() {
    printf("[%zu] Started threads: %d, ended threads: %d\n", get_time(), startedThreads, endedThreads);
}

void started_threads_incre() {
    pthread_mutex_lock(&mutex);
    startedThreads++;
    pthread_mutex_unlock(&mutex);
}

void ended_threads_incre() {
    pthread_mutex_lock(&mutex);
    endedThreads++;
    pthread_mutex_unlock(&mutex);
}

__time_t get_time() {
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) == -1) {
        printf("clock_gettime error\n");
        exit(EXIT_FAILURE);
    }

    return time.tv_nsec / 1000;
}

void clean_after() {
    if (pthread_mutex_destroy(&mutex) < 0) {
        perror("pthread_mutex_destroy error");
        exit(EXIT_FAILURE);
    }
}

void *action(void *args) {
    thread_args *params = (thread_args *) args;

    started_threads_incre();
    printf("[%zu] (id: %d) starting thread with TID %ld\n", get_time(), params->id, pthread_self());
    sleep(SUSP_TIME);
    printf("[%zu] (id: %d) end thread with TID %ld after sleeping\n", get_time(), params->id, pthread_self());
    ended_threads_incre();

    return NULL;
}

void *gen_err_action(void *args) {
    started_threads_incre();
    printf("[%zu] starting gen_err_action with TID %ld\n", get_time(), pthread_self());

    int a = 5;
    a = a / 0;
    printf("%d\n", a);

    printf("[%zu] end gen_err_action with TID %ld\n", get_time(), pthread_self());
    ended_threads_incre();

    return NULL;
}

void make_threads() {
    for (int i=0; i<THREADS_NUM; i++) {
        thread_args *params = calloc(1, sizeof(thread_args));
        params->id = i+1;
        if (pthread_create(&pthreads[i], NULL, &action, (void *) params) == -1) {
            perror("creating thread error");
            exit(EXIT_FAILURE);
        }
    }

    pthread_t err_thread;
    pthread_create(&err_thread, NULL, &gen_err_action, NULL);

    for (int i=0; i<THREADS_NUM; i++) {
        pthread_join(pthreads[i], NULL);
    }
    pthread_join(err_thread, NULL);
}

void init() {
    pthreads = (pthread_t *) calloc((size_t) THREADS_NUM, sizeof(pthread_t));

    if (pthread_mutex_init(&mutex, NULL) < 0) {
        perror("pthread_mutex_init error");
        exit(EXIT_FAILURE);
    }
}
