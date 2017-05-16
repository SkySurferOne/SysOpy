#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define CASE1 1
#define CASE2 2
#define CASE3 3
#define CASE4 4
#define CASE5 5

#define SUSP_TIME 2

// structs
typedef struct ParsedArgs {
    char *exePath;
    int threadNum;
    int caseNum;
    int sig;
} ParsedArgs;

typedef struct thead_args {
    int id;
} thread_args;

// api
void clean_after();
void parse_args(int, char **);
void init();
void make_threads();
void *action(void *);
void started_threads_incre();
void ended_threads_incre();
void print_summary();
void sig_handler(int);
__time_t get_time();

// globals
ParsedArgs *parsedArgs;
pthread_t *pthreads;
static int startedThreads = 0, endedThreads = 0;
pthread_mutex_t mutex;
const int signals[4] = {SIGUSR1, SIGTERM, SIGKILL, SIGSTOP};
const char* signals_str[] = {"SIGUSR1", "SIGTERM", "SIGKILL", "SIGSTOP"};
int csignal;

int main(int argc, char **argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    printf("[%zu] params: threads %d, signal: %s, case: %d\n",
           get_time(), parsedArgs->threadNum, signals_str[parsedArgs->sig-1], parsedArgs->caseNum);
    init();
    make_threads();

    print_summary();
    clean_after();
    return 0;
}

void sig_handler(int signo) {
    printf("[%zu] PID = %d TID = %d\n", get_time(), getpid(), (int) pthread_self());
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

void send_sig() {
    if(parsedArgs->caseNum == CASE4 || parsedArgs->caseNum == CASE5) {
        for (int i=0; i<parsedArgs->threadNum; i++) {
            if (pthread_kill(pthreads[i], csignal) < 0) {
                printf("pthread_kill error\n");
                exit(EXIT_FAILURE);
            }
            printf("[%zu] Send signal %s to the (%d) thread.\n", get_time(), signals_str[parsedArgs->sig-1], i+1);
        }
        printf("[%zu] Send signals to the threads.\n", get_time());
    }  // CASE1 and CASE2
    else {
        if (kill(getpid(), csignal) < 0) {
            printf("kill error\n");
            exit(EXIT_FAILURE);
        }
        printf("[%zu] Send signal %s to the main process\n", get_time(), signals_str[parsedArgs->sig-1]);
    }
}

void clean_after() {
    free(parsedArgs);

    if (pthread_mutex_destroy(&mutex) < 0) {
        perror("pthread_mutex_destroy error");
        exit(EXIT_FAILURE);
    }
}

void *action(void *args) {
    thread_args *params = (thread_args *) args;
    if(parsedArgs->caseNum == CASE3 || parsedArgs->caseNum == CASE5)
        signal(csignal, sig_handler);

    if(parsedArgs->caseNum == CASE4) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, csignal);

        if (sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
            perror("sigprocmask error");
            exit(EXIT_FAILURE);
        }
        printf("[%zu] Running thread %d with sigprocmask for %s\n",
               get_time(), params->id, signals_str[parsedArgs->sig-1]);
    }

    started_threads_incre();
    printf("[%zu] (id: %d) starting thread with TID %ld\n", get_time(), params->id, pthread_self());
    sleep(SUSP_TIME);
    printf("[%zu] (id: %d) end thread with TID %ld after sleeping\n", get_time(), params->id, pthread_self());
    ended_threads_incre();
    return NULL;
}

void make_threads() {
    for (int i=0; i<parsedArgs->threadNum; i++) {
        thread_args *params = calloc(1, sizeof(thread_args));
        params->id = i+1;
        if (pthread_create(&pthreads[i], NULL, &action, (void *) params) == -1) {
            perror("creating thread error");
            exit(EXIT_FAILURE);
        }
    }

    send_sig();

    for (int i=0; i<parsedArgs->threadNum; i++) {
        pthread_join(pthreads[i], NULL);
    }
}

void init() {
    pthreads = (pthread_t *) calloc((size_t) parsedArgs->threadNum, sizeof(pthread_t));

    if (pthread_mutex_init(&mutex, NULL) < 0) {
        perror("pthread_mutex_init error");
        exit(EXIT_FAILURE);
    }

    if(parsedArgs->caseNum == CASE2) {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, csignal);
        if (sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
            perror("sigprocmask error");
            exit(EXIT_FAILURE);
        }
        printf("[%zu] Set sigprocmask for %s in main process\n", get_time(), signals_str[parsedArgs->sig-1]);
    }

    if(parsedArgs->caseNum == CASE3 || parsedArgs->caseNum == CASE5)
        signal(csignal, sig_handler);
}

void parse_args(int argc, char **argv) {
    if (argc < 2) {
        printf("To less arguments.\n"
               "Syntax: <thread_num> <case_num [1;5]> <sig_num [1;4]{SIGUSR1, SIGTERM, SIGKILL, SIGSTOP}>\n");
        exit(EXIT_FAILURE);
    }
    parsedArgs->exePath = argv[0];
    parsedArgs->threadNum = atoi(argv[1]);
    parsedArgs->caseNum = atoi(argv[2]);
    parsedArgs->sig = atoi(argv[3]);

    if (parsedArgs->caseNum < 1 || parsedArgs->caseNum > 5) {
        perror("wrong case number");
        exit(EXIT_FAILURE);
    }

    if (parsedArgs->sig < 1 || parsedArgs->sig > 4) {
        perror("wrong singal number");
        exit(EXIT_FAILURE);
    }
    csignal = signals[parsedArgs->sig-1];
}