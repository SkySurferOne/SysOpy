#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define range(x) for(int i=0;i<x;i++)

typedef struct ParsedArgs {
    char *exePath;
    int sigNo;
    int type;
} ParsedArgs;

void parse_args(int, char **);
void config();
void create_child();
void wait_for_pid(pid_t);
void send_signal(pid_t);
void get_signals(int, siginfo_t *, void *);
void get_signals_ancestor(int, siginfo_t *, void *);
void kill_em_and_leave(pid_t);

// globals
ParsedArgs *parsedArgs;
int sigusr1ReceivedByChild = 0, sigusr1ReceivedByAncestor = 0;

int main(int argc, char ** argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    config();
    create_child();

    // show numbers
    char *str = (parsedArgs->type == 3) ? "SIGRTMIN" : "SIGUSR1";
    printf("Ancestor received %d singals (%s) from child\n", sigusr1ReceivedByAncestor, str);

    free(parsedArgs);
    return 0;
}

void send_signal(pid_t pid) {
    union sigval val;

    switch (parsedArgs->type) {
        case 1:
            kill(pid, SIGUSR1);
            break;
        case 2:
            val.sival_int = SIGUSR1;
            if(sigqueue(pid, SIGUSR1, val) != 0){
                printf("Something went wrong in sigqueue function\n");
            }
            break;
        case 3:
            kill(pid, SIGRTMIN);
            break;
        default:
            printf("Wrong type number\n");
    }

}

void kill_em_and_leave(pid_t pid) {
    union sigval val;

    switch (parsedArgs->type) {
        case 1:
            kill(pid, SIGUSR2);
            break;
        case 2:
            val.sival_int = SIGUSR2;
            if(sigqueue(pid, SIGUSR2, val) != 0){
                printf("Something went wrong in sigqueue function\n");
            }
            break;
        case 3:
            kill(pid, SIGRTMIN+1);
            break;
        default:
            printf("Wrong type number\n");
    }
}

void get_signals(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGUSR1 || sig == SIGRTMIN) {
        sigusr1ReceivedByChild++;
        send_signal(getppid());
    } else if (sig == SIGUSR2 || sig == (SIGRTMIN + 1)) {
        char *str = (parsedArgs->type == 3) ? "SIGRTMIN" : "SIGUSR1";
        printf("Child received %d signals (%s) from ancestor\n", sigusr1ReceivedByChild, str);
        fflush(stdout);
        _exit(0);
    }
}

void get_signals_ancestor(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGUSR1 || sig == SIGRTMIN) {
        sigusr1ReceivedByAncestor++;
    }
}

void config() {
    struct sigaction act;
    act.sa_sigaction = &get_signals_ancestor; // receive usr1
    act.sa_flags = SA_SIGINFO | SA_RESTART;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGRTMIN, &act, NULL);
}

void create_child() {
    pid_t pid;
    char *str = (parsedArgs->type == 3) ? "SIGRTMIN" : "SIGUSR1";

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {

        struct sigaction act;
        act.sa_sigaction = &get_signals; // receive usr1 & usr2 sig
        act.sa_flags = SA_SIGINFO;

        sigaction(SIGUSR1, &act, NULL);
        sigaction(SIGUSR2, &act, NULL);
        sigaction(SIGRTMIN, &act, NULL);
        sigaction(SIGRTMIN + 1, &act, NULL);

        while (1) {}

    } else {
        sleep(1);
        range(parsedArgs->sigNo) {
            send_signal(pid);
        }
        printf("%d signals (%s) sent to child\n", parsedArgs->sigNo, str);
        fflush(stdout);
        kill_em_and_leave(pid);

        wait_for_pid(pid);
    }
}

void wait_for_pid(pid_t pid) {
    pid_t w;
    int status;

    w = waitpid(pid, &status, 0);

    printf("\tAfter waiting waitpid ret: >>%d<<, status: %d\n", w, status);
    fflush(stdout);
    if (WIFEXITED(status)) {
        printf("\texited, status=%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("\tkilled by signal %d\n", WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        printf("\tstopped by signal %d\n", WSTOPSIG(status));
    } else if (WIFCONTINUED(status)) {
        printf("\tcontinued\n");
    }
}

void parse_args(int argc, char ** argv) {
    if (argc < 3) { printf("To less arguments. Syntax: <sigNo> <type> \n"); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->sigNo = atoi(argv[1]);
    parsedArgs->type = atoi(argv[2]);
    if (parsedArgs->sigNo < 0) { printf("Has to be a positive number\n"); exit(1); }
    if (parsedArgs->type < 1 || parsedArgs->type > 3) { printf("Wrong type parameter\n"); exit(1); }
}
