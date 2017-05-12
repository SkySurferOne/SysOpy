#include <zconf.h>
#include "common.h"


// structs
typedef struct ParsedArgs {
    char *exePath;
    int clientsNum;
    int cutsNum;
} ParsedArgs;

// api
void parse_args(int, char **);
void initialize_res();
void config();
void sigint_handler(int, siginfo_t *, void *);
void make_clients();
void goto_barber();
int try_to_get_haircut();
void clean();

// globals
ParsedArgs *parsedArgs;
int sid, smid, * smaddr, cutsCounter = 0;

int main(int argc, char **argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    config();
    make_clients();

    free(parsedArgs);
    return 0;
}

void sigint_handler(int sig, siginfo_t *siginfo, void *context) {
    printf("Received sigint. Closing barber.\n");
    exit(EXIT_SUCCESS);
}

void parse_args(int argc, char **argv) {
    if (argc < 3) {
        printf("To less arguments. Syntax: <num_of_clients> <cuts_num>\n");
        exit(EXIT_FAILURE);
    }
    parsedArgs->exePath = argv[0];
    parsedArgs->clientsNum = atoi(argv[1]);
    parsedArgs->cutsNum = atoi(argv[2]);
}

void config() {
    // handle signals
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &sigint_handler;
    act.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &act, NULL) == -1) {
        printf("sigaction error\n");
        exit(EXIT_FAILURE);
    }

    initialize_res();
}

void make_clients() {
    int clientsCounter = 0, status;

    for(int i = 0; i < parsedArgs->clientsNum; i++) {
        if(fork() == 0) {
            goto_barber();
            _exit(EXIT_SUCCESS);
        }
    }

    while(clientsCounter < parsedArgs->clientsNum) {
        wait(&status);
        clientsCounter++;

        if(status == EXIT_SUCCESS) {
            clientsCounter++;
        }
        else {
            printf("child error\n");
            exit(EXIT_SUCCESS);
        }
    }
}

int try_to_get_haircut() {
    int status;
    take_semaphore(sid, FIFO_SEM_NUM);
    int barberSemVal = semctl(sid, BARBER_SEM_NUM, GETVAL);
    if(barberSemVal == -1) {
        printf("getting barber semaphore value error\n");
        exit(EXIT_FAILURE);
    }

    if(barberSemVal == 0) {
        printf(ANSI_COLOR_GREEN "[%zu] CLIENT (%d): Waking up barber."
                       ANSI_COLOR_RESET "\n", get_time(), getpid());
        give_semaphore(sid, BARBER_SEM_NUM);
        give_semaphore(sid, BARBER_SEM_NUM);
        bqueue_occupy_chair(smaddr, getpid());
        status = 0;
    } else {
        if(bqueue_put(smaddr, getpid()) == -1) {
            printf(ANSI_COLOR_YELLOW "[%zu] CLIENT (%d): Barber institution is full. Going away."
                           ANSI_COLOR_RESET "\n", get_time(), getpid());
            status = -1;
        } else {
            printf(ANSI_COLOR_BLUE "[%zu] CLIENT (%d): Barber is working. Taking seat in the waiting room."
                           ANSI_COLOR_RESET "\n", get_time(), getpid());
            status = 0;
        }
    }

    give_semaphore(sid, FIFO_SEM_NUM);
    return status;
}

void clean() {
    if(shmdt(smaddr) == -1) {
        printf("shmdt error\n");
        exit(EXIT_FAILURE);
    }
}

void goto_barber() {
    while(cutsCounter < parsedArgs->cutsNum) {
        int status = try_to_get_haircut();

        if (status == 0) {
            take_semaphore(sid, CUT_SEM_NUM);
            cutsCounter++;
            printf(ANSI_COLOR_MAGENTA "[%zu] CLIENT (%d): Having hair cut."
                           ANSI_COLOR_RESET "\n", get_time(), getpid());
        }
    }

    clean();
}

void initialize_res() {
    key_t key = ftok(getenv("HOME"), SEMG_KEY);

    sid = semget(key, 0, 0);
    if(sid == -1) {
        printf("semget error\n");
        exit(EXIT_FAILURE);
    }

    // get id of shared memorys
    int smid = shmget(key, 0, 0);
    if(smid == -1) {
        printf("shmget error\n");
        exit(EXIT_FAILURE);
    }

    // include memory segment to process address space
    smaddr = shmat(smid, NULL, 0);
    if(smaddr == (void *) -1) {
        printf("shmat error\n");
        exit(EXIT_FAILURE);
    }
}