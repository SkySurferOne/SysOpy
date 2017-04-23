#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <string.h>

#define range(num) for (int i=0; i<num; i++)
#define NUM_SLAVES 1
#define NUM_CHILD NUM_SLAVES+1

typedef struct ParsedArgs {
    char *exePath,
    *path, // path to fifo
    *N, // number of points
    *K, // number of iterations
    *R; // size of array (T) which represents screen (pixels)
} ParsedArgs;

// globals
int fifoIsReady = 0;
pid_t masterPid;
int slavesReadyCounter = NUM_SLAVES;
pid_t children[NUM_CHILD];

void parse_args(int, char **, ParsedArgs *);
void config();
void sig_handler(int, siginfo_t *, void *);
void start_master(char *, char *);
void start_slaves(char *, char *, char *);
void wait_for_children();
void show_help();

int main(int argc, char **argv) {
    config();
    ParsedArgs *parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv, parsedArgs);
    start_master(parsedArgs->path, parsedArgs->R);
    start_slaves(parsedArgs->path, parsedArgs->N, parsedArgs->K);

    wait_for_children();
    free(parsedArgs);
    return 0;
}

void parse_args(int argc, char **argv, ParsedArgs *parsedArgs) {
    if (argc < 5) { printf("To less arguments\n"); show_help(); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->path = argv[1];
    parsedArgs->N =  argv[2];
    parsedArgs->K =  argv[3];
    parsedArgs->R = argv[4];

    long N = strtol(parsedArgs->N, NULL, 10);
    int K = (int) strtol(parsedArgs->K, NULL, 10);
    int R = (int) strtol(parsedArgs->R, NULL, 10);
    if (N < 100 || N > 2000000000) {
        printf("Incorrect N value\n");
        exit(EXIT_FAILURE);
    }
    if (K < 0 || K > 10000) {
        printf("Incorrect K value\n");
        exit(EXIT_FAILURE);
    }
    if (R < 50 || R > 2000) {
        printf("Incorrect R value\n");
        exit(EXIT_FAILURE);
    }
}

void config() {
    struct sigaction act;

    act.sa_sigaction = &sig_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;

    sigaction(SIGRTMIN, &act, NULL); // fifo is ready sig
    sigaction(SIGRTMIN+1, &act, NULL); // slaves are ready
    sigaction(SIGRTMIN+2, &act, NULL); // sth went wrong
}

void sig_handler(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGRTMIN) {
        printf("Fifo is ready\n");
        // fifo is ready
        fifoIsReady = 1;
    }
    else if (sig == SIGRTMIN + 1) {
        printf("Main: Slave is ready\n");
        // slaves are ready, master can start reading form fifo
        if (--slavesReadyCounter == 0)
            kill(masterPid, SIGRTMIN + 1);
    }
    else if (sig == SIGRTMIN + 2) {
        printf("Something went wrong\n");
        exit(1);
        // sth went wrong, end main and kill children
    }
}

void start_master(char *path, char *R) {
    char *argv[4];
    pid_t pid;

    argv[0] = "./master";
    argv[1] = path;
    argv[2] = R;
    argv[3] = NULL;

    pid = fork();
    if (pid < 0) {
        printf("Fork error\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        int res = execvp(argv[0], argv);
        if(res == -1) abort(); else exit(res);
    } else {
        masterPid = pid;
        children[0] = masterPid;
        printf("Master pid: %d\n", masterPid);
    }

    while(fifoIsReady != 1) { sleep(1); }
}

void start_slaves(char *path, char *N, char *K) {
    char *argv[5];
    pid_t pid;

    argv[0] = "./slave";
    argv[1] = path;
    argv[2] = N;
    argv[3] = K;
    argv[4] = NULL;

    range(NUM_SLAVES) {
        pid = fork();
        if (pid < 0) {
            printf("Fork error\n");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            int res = execvp(argv[0], argv);
            if(res == -1) abort(); else exit(res);
        } else {
            children[i+1] = pid;
        }
    }

}

void wait_for_children() {
    int status;
    volatile pid_t w;
    range(NUM_CHILD) {
        w = waitpid(children[i], &status, 0); // wait for any child process

        if (status != 0) {
            printf("After waiting waitpid ret: >>%d<<, status: %d\n", w, status);
            if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }

            printf("Command termination failed - Error %d", status);
            exit(EXIT_FAILURE);
        }
    }
}

void show_help() {
    printf("syntax: path N K R\n"
                   "where:\n"
                   "'path' is a path to named pipe (FIFO)\n"
                   "'N' number of randomly choosen complex points\n"
                   "'K' number of iterations\n"
                   "'R' number of 'pixels'\n");
}

