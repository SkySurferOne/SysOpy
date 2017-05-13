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
pid_t *smaddr;
sem_t *semarr[3];
int breakWorkLoop = 0;

int main(int argc, char **argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    config();
    work_loop();

    free(parsedArgs);
    return 0;
}

void clean() {
    if (munmap(smaddr, (parsedArgs->chairsNum + 4) * sizeof(int)) == -1) {
        printf("munmap error\n");
    }

    if(shm_unlink(MEMORY_NAME) == -1) {
        printf("shm_unlink error\n");
    }

    if(sem_unlink(BARBER_NAME) == -1 ||
       sem_unlink(FIFO_NAME) == -1 ||
       sem_unlink(CUT_NAME) == -1) {
        printf("sem_unlink error\n");
    }
}

void sigint_handler(int sig, siginfo_t *siginfo, void *context) {
    printf("Received sigint. Closing barber.\n");
    breakWorkLoop = 1;
    // exit(EXIT_SUCCESS);
}

void initialize_res() {
    // Open a named semaphores
    semarr[BARBER_SEM_NUM] = sem_open(BARBER_NAME, O_CREAT | O_RDWR | O_EXCL, 0600, 0);
    semarr[FIFO_SEM_NUM] = sem_open(FIFO_NAME, O_CREAT | O_RDWR | O_EXCL, 0600, 1);
    semarr[CUT_SEM_NUM] = sem_open(CUT_NAME, O_CREAT | O_RDWR | O_EXCL, 0600, 0);

    if(semarr[BARBER_SEM_NUM] == SEM_FAILED ||
       semarr[FIFO_SEM_NUM] == SEM_FAILED ||
       semarr[CUT_SEM_NUM] == SEM_FAILED) {
        printf("sem_open error: cannot retrieve semaphore\n");
        exit(EXIT_FAILURE);
    }

    int smid = shm_open(MEMORY_NAME, O_CREAT | O_RDWR | O_EXCL, 0600);
    if(smid == -1) {
        printf("shm_open error\n");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(smid, (parsedArgs->chairsNum + 4) * sizeof(pid_t)) == -1) {
        printf("ftruncate error\n");
        exit(EXIT_FAILURE);
    }

    smaddr = mmap(NULL, (parsedArgs->chairsNum + 4) * sizeof(pid_t), PROT_READ | PROT_WRITE, MAP_SHARED, smid, 0);
    if(smaddr == (void*)-1) {
        printf("mmap error\n");
        exit(EXIT_FAILURE);
    }

    bqueue_init(smaddr, parsedArgs->chairsNum);
}

void make_haircut(int clientPid) {
    printf(ANSI_COLOR_GREEN "[%zu] BARBER: Making haircut for %d." ANSI_COLOR_RESET "\n", get_time(), clientPid);
    sem_post(semarr[CUT_SEM_NUM]);
    printf(ANSI_COLOR_YELLOW "[%zu] BARBER: Finished haircut for %d." ANSI_COLOR_RESET "\n", get_time(), clientPid);
}

void work_loop() {
    pid_t clientPid;

    printf(ANSI_COLOR_BLUE "[%zu] BARBER: Is sleeping." ANSI_COLOR_RESET "\n", get_time());
    while(!breakWorkLoop) {
        sem_wait(semarr[BARBER_SEM_NUM]);
        sem_wait(semarr[FIFO_SEM_NUM]);
        clientPid = bqueue_get_customer_from_chair(smaddr);
        sem_post(semarr[FIFO_SEM_NUM]);

        make_haircut(clientPid);

        while(1) {
            sem_wait(semarr[FIFO_SEM_NUM]);
            bqueue_show(smaddr);
            clientPid = bqueue_get(smaddr);

            if(clientPid != -1) {
                bqueue_occupy_chair(smaddr, clientPid);
                make_haircut(clientPid);
                sem_post(semarr[FIFO_SEM_NUM]);
            }
            else {
                printf(ANSI_COLOR_BLUE "[%zu] BARBER: Is sleeping." ANSI_COLOR_RESET "\n", get_time());
                sem_wait(semarr[BARBER_SEM_NUM]);
                sem_post(semarr[FIFO_SEM_NUM]);
                break;
            }
        }
    }

    clean();
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