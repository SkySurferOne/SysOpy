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
sem_t *semarr[3];
int cutsCounter = 0, *smaddr, smsize;

int main(int argc, char **argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    config();
    make_clients();

    free(parsedArgs);
    return 0;
}

void sigint_handler(int sig, siginfo_t *siginfo, void *context) {
    printf("Received sigint. Closing client.\n");
    clean();
    exit(EXIT_SUCCESS);
}

int try_to_get_haircut() {
    sem_wait(semarr[FIFO_SEM_NUM]);
    int status;
    int barberSemVal;

    sem_getvalue(semarr[BARBER_SEM_NUM], &barberSemVal);
    if(barberSemVal == -1) {
        printf("getting barber semaphore value error\n");
        exit(EXIT_FAILURE);
    }

    if(barberSemVal == 0) {
        printf(ANSI_COLOR_GREEN "[%zu] CLIENT (%d): Waking up barber."
                       ANSI_COLOR_RESET "\n", get_time(), getpid());
        sem_post(semarr[BARBER_SEM_NUM]);
        sem_post(semarr[BARBER_SEM_NUM]);
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

    sem_post(semarr[FIFO_SEM_NUM]);
    return status;

}

void clean() {
    if(sem_close(semarr[BARBER_SEM_NUM]) == -1 ||
       sem_close(semarr[FIFO_SEM_NUM]) == -1 ||
       sem_close(semarr[CUT_SEM_NUM]) == -1 ) {
        printf("sem_close error\n");
        exit(EXIT_FAILURE);
    }

    if(munmap(smaddr, smsize * sizeof(int)) == -1) {
        printf("munmap error\n");
        exit(EXIT_FAILURE);
    }
}

void goto_barber() {
    while(cutsCounter < parsedArgs->cutsNum) {
        int status = try_to_get_haircut();

        if (status == 0) {
            sem_wait(semarr[CUT_SEM_NUM]);
            ++cutsCounter;
            printf(ANSI_COLOR_MAGENTA "[%zu] CLIENT (%d): Having hair cut."
                           ANSI_COLOR_RESET "\n", get_time(), getpid());
        }
    }
    clean();
}

void initialize_res() {
    // Open a named semaphores
    semarr[BARBER_SEM_NUM] = sem_open(BARBER_NAME, O_RDWR, 0600, 0);
    semarr[CUT_SEM_NUM] = sem_open(CUT_NAME, O_RDWR, 0600, 0);
    semarr[FIFO_SEM_NUM] = sem_open(FIFO_NAME, O_RDWR, 0600, 0);

    if(semarr[BARBER_SEM_NUM] == SEM_FAILED ||
       semarr[FIFO_SEM_NUM] == SEM_FAILED ||
       semarr[CUT_SEM_NUM] == SEM_FAILED) {
        printf("sem_open error: cannot retrieve semaphore\n");
        exit(EXIT_FAILURE);
    }

    // get id of shared memory
    int smid = shm_open(MEMORY_NAME, O_RDWR, 0600);
    if(smid == -1) {
        printf("shm_open error\n");
        exit(EXIT_FAILURE);
    }

    smaddr = mmap(NULL, sizeof(int), PROT_READ, MAP_SHARED, smid, 0);
    if(smaddr == (void *) -1) {
        printf("mmap error\n");
        exit(EXIT_FAILURE);
    }

    smsize = smaddr[0];

    if(munmap(smaddr, sizeof(int)) == -1) {
        printf("munmap error\n");
        exit(EXIT_FAILURE);
    }

    smaddr = mmap(NULL, smsize * sizeof(int), PROT_WRITE | PROT_READ, MAP_SHARED, smid, 0);
    if(smaddr == (void *) -1) {
        printf("mmap error\n");
        exit(EXIT_FAILURE);
    }
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
            // printf("pid: %d\n", p);
        }
        else {
            printf("child error\n");
            exit(EXIT_SUCCESS);
        }
    }
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

void parse_args(int argc, char **argv) {
    if (argc < 3) {
        printf("To less arguments. Syntax: <num_of_clients> <cuts_num>\n");
        exit(EXIT_FAILURE);
    }
    parsedArgs->exePath = argv[0];
    parsedArgs->clientsNum = atoi(argv[1]);
    parsedArgs->cutsNum = atoi(argv[2]);
}