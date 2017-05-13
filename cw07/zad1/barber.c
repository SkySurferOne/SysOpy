#include "common.h"


// structs
typedef struct ParsedArgs {
    char *exePath;
    int chairsNum;
} ParsedArgs;

// api
void config();
void parse_args(int, char **);
void clean();
void sigint_handler(int, siginfo_t *, void *);
void initialize_res();
void work_loop();

// globals
ParsedArgs *parsedArgs;
int sid, smid, * smaddr, breakWorkLoop = 0;


int main(int argc, char ** argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    config();
    work_loop();

    free(parsedArgs);
    return 0;
}

void clean() {
    if (shmdt(smaddr) == -1) {
        printf("shmdt error\n");
    }

    if(shmctl(smid, IPC_RMID,NULL) == -1) {
        printf("shmctl error\n");
    }

    if(semctl(sid, 0, IPC_RMID) == -1) {
        printf("semctl error: problem with deleting semaphores\n");
    }
}

void make_haircut(int clientPid) {
    printf(ANSI_COLOR_GREEN "[%zu] BARBER: Making haircut for %d." ANSI_COLOR_RESET "\n", get_time(), clientPid);
    give_semaphore(sid, CUT_SEM_NUM);
    printf(ANSI_COLOR_YELLOW "[%zu] BARBER: Finished haircut for %d." ANSI_COLOR_RESET "\n", get_time(), clientPid);
}

void work_loop() {
    pid_t clientPid;

    printf(ANSI_COLOR_BLUE "[%zu] BARBER: Is sleeping." ANSI_COLOR_RESET "\n", get_time());
    while(!breakWorkLoop) {
        take_semaphore(sid, BARBER_SEM_NUM);
        take_semaphore(sid, FIFO_SEM_NUM);
        clientPid = bqueue_get_customer_from_chair(smaddr);
        give_semaphore(sid, FIFO_SEM_NUM);

        make_haircut(clientPid);

        while(1) {
            take_semaphore(sid, FIFO_SEM_NUM);
            bqueue_show(smaddr);
            clientPid = bqueue_get(smaddr);

            if(clientPid != -1) {
                bqueue_occupy_chair(smaddr, clientPid);
                make_haircut(clientPid);
                give_semaphore(sid, FIFO_SEM_NUM);
            }
            else {
                printf(ANSI_COLOR_BLUE "[%zu] BARBER: Is sleeping." ANSI_COLOR_RESET "\n", get_time());
                take_semaphore(sid, BARBER_SEM_NUM);
                give_semaphore(sid, FIFO_SEM_NUM);
                break;
            }
        }
    }

    clean();
}

void sigint_handler(int sig, siginfo_t *siginfo, void *context) {
    printf("Received sigint. Closing barber.\n");
    breakWorkLoop = 1;
    // exit(EXIT_SUCCESS);
}

void initialize_res() {
    key_t key = ftok(getenv("HOME"), SEMG_KEY);

    sid = semget(key, 3, IPC_CREAT | 0600);
    if(sid == -1) {
        printf("semget error\n");
        exit(EXIT_FAILURE);
    }

    if(semctl(sid, BARBER_SEM_NUM, SETVAL, 0) == -1 ||
       semctl(sid, FIFO_SEM_NUM, SETVAL, 1) == -1 ||
       semctl(sid, CUT_SEM_NUM, SETVAL, 0) == -1) {
        printf("semctl error\n");
        exit(EXIT_FAILURE);
    }

    // get id of shared memory
    smid = shmget(key, (parsedArgs->chairsNum + 4) * sizeof(pid_t), IPC_CREAT | 0600);
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

    bqueue_init(smaddr, parsedArgs->chairsNum);
}

void config() {
    // clean after normal process termination
    atexit(clean);

    // handle signals
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = &sigint_handler;
    act.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &act, NULL) == -1) {
        printf("sigaction error\n");
        exit(EXIT_FAILURE);
    }

    // initialize semaphores and shared memory
    initialize_res();
}

void parse_args(int argc, char **argv) {
    if (argc < 2) {
        printf("To less arguments. Syntax: <num_of_chairs>\n");
        exit(EXIT_FAILURE);
    }
    parsedArgs->exePath = argv[0];
    parsedArgs->chairsNum = atoi(argv[1]);
}