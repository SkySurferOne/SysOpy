#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <wait.h>
#include <string.h>

#define range(x) for(int i=0;i<x;i++)
#define SLEEP_MAX_SEC 10
#define DO_ASYNC 0

typedef struct ParsedArgs {
    char *exePath;
    int childNo;
    int reqNo;
} ParsedArgs;

void parse_args(int, char **);
void config();
void create_children();
void simulate_work();
void send_request();
void receive_request(int, siginfo_t *, void *);
void get_permission(int, siginfo_t *, void *);
void send_permission(pid_t);
void receive_request(int, siginfo_t *, void *);
void start_clock();
float stop_clock();
void terminate_program(int, siginfo_t *, void *);
void wait_for_pid(pid_t);

// globals
ParsedArgs *parsedArgs;
pid_t *suspendedProc = NULL, *allChildren = NULL, PPID, PCP;
int receivedReq = 0, susp;
clock_t start, end;

int main(int argc, char ** argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    config();

    free(parsedArgs);
    return 0;
}

void terminate_program(int sig, siginfo_t *siginfo, void *context) {
    if (allChildren) {
        range(parsedArgs->childNo) {
            if (allChildren[i]) kill(allChildren[i], SIGSTOP);
        }
    }

    exit(SIGINT);
}

void create_children() {
    pid_t pid;

    range(parsedArgs->childNo) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            printf("\t>>%d<< started working\n", getpid());

            simulate_work();

            start_clock();
            send_request();
            if(pause() == -1) {
                printf("GET PERMISSION signal was caught by <<%d>>\n", getpid());
            }
            printf("Time: %f [sec]\n", stop_clock());

            _exit(0);

        } else {
            allChildren[i] = pid;

            if (i + 1 == parsedArgs->reqNo) PCP = pid;
            if (i + 1>= parsedArgs->reqNo) {
                wait_for_pid(pid);
            } else {
                susp = 1;
                while(susp){}
            }

        }
    }

}

void wait_for_pid(pid_t pid) {
    pid_t w;
    int status;

    if (DO_ASYNC)
        w = waitpid(-1, &status, 0);
    else
        w = waitpid(pid, &status, 0);


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
}

inline void start_clock() {
    start = clock();
}

inline float stop_clock() {
    end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    return seconds;
}


void real_time_handler(int sig, siginfo_t *siginfo, void *context) {
    printf("<<%d>> received %d sig from >>%d<<\n", getpid(), sig, siginfo->si_pid);
}

void get_permission(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGUSR2) {
        printf(">>%d<< received permission\n", getpid());
        srand(time(NULL));
        int sig = rand() % 3 + SIGRTMIN + 1;
        printf(">>%d<< send %d sig to <<%d>>\n", getpid(), sig, getppid());
        kill(getppid(), sig);
    }
}

void receive_request(int sig, siginfo_t *siginfo, void *context) {
    printf("<<%d>> received request from >>%d<<\n", getpid(), siginfo->si_pid);
    if (receivedReq < parsedArgs->reqNo)
        suspendedProc[receivedReq] = siginfo->si_pid;
    receivedReq++;

    susp = 0;
    if (receivedReq == parsedArgs->reqNo) {
        range(parsedArgs->reqNo) {
            send_permission(suspendedProc[i]);
            // wait
            if(siginfo->si_pid != PCP)
                wait_for_pid(suspendedProc[i]);
        }
    }

    if (receivedReq > parsedArgs->reqNo) {
        send_permission(siginfo->si_pid);
    }

}

void send_permission(pid_t pid) {
    printf("<<%d>> send permission (SIGUSR2) to >>%d<<\n", getpid(), pid);
    kill(pid, SIGUSR2);
}

void send_request() {
    printf(">>%d<< sending request (SIGUSR1) to <<%d>>\n", getpid(), getppid());
    kill(getppid(), SIGUSR1);
}

void simulate_work() {
    srand(time(NULL));
    int t = rand() % (SLEEP_MAX_SEC) + 1;
    printf("Child process simulate work (pid: >>%d<<, dur: %d [sec])\n", getpid(), t);
    sleep((unsigned int) t);
}

void config() {
    PPID = getpid();
    printf("Mother process pid: <<%d>>\n", getpid());
    printf("SIGRTMIN: %d, SIGRTMAX: %d\n", SIGRTMIN, SIGRTMAX);
    suspendedProc = malloc(sizeof(pid_t) * parsedArgs->reqNo);
    allChildren = malloc(sizeof(pid_t) * parsedArgs->childNo);
    memset(suspendedProc, 0, sizeof(pid_t) * parsedArgs->reqNo);
    memset(allChildren, 0, sizeof(pid_t) * parsedArgs->childNo);

    struct sigaction act;
    memset(&act, 0, sizeof(act));

    act.sa_sigaction = &receive_request;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_sigaction = &get_permission;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act, NULL);

    act.sa_sigaction = real_time_handler;
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGRTMIN + 1, &act, NULL);
    sigaction(SIGRTMIN + 2, &act, NULL);
    sigaction(SIGRTMIN + 3, &act, NULL);

    act.sa_sigaction = &terminate_program;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);

    create_children();
    free(suspendedProc);
}

void parse_args(int argc, char ** argv) {
    if (argc < 3) { printf("To less arguments. Syntax: <child_no> <req_no> \n"); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->childNo = atoi(argv[1]);
    parsedArgs->reqNo = atoi(argv[2]);
    if (parsedArgs->reqNo > parsedArgs->childNo) { printf("reqNo cannot be greater than childNo\n"); exit(1); }
}