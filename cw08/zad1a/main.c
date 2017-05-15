#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define RECORD_SIZE 1024
#define NOT_FOUND -3

// structs
typedef struct ParsedArgs {
    char *exePath;
    int threadNum;
    char *fileName;
    int recordNum;
    char *word;
} ParsedArgs;

typedef struct record {
    int id;
    char text[RECORD_SIZE];
} record;

// globals
ParsedArgs *parsedArgs;
int fileDesc;
pthread_t * pthreads;
static pthread_key_t thread_log_key;
pthread_mutex_t mutex;

// api
void parse_args(int, char **);
void init();
void make_threads();
void *parse(void *);
void clean_after();

int main(int argc, char **argv) {
    parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    init();
    make_threads();

    clean_after();
    return 0;
}

void clean_after() {
    close(fileDesc);
    free(parsedArgs);
    if (pthread_mutex_destroy(&mutex) < 0) {
        perror("pthread_mutex_destroy error");
        exit(EXIT_FAILURE);
    }
}

int find_word() {
    char * record = (char *) calloc((size_t) RECORD_SIZE, sizeof(char));
    pthread_setspecific(thread_log_key, record);

    int process = 1, size = 0;
    for (int i=0; i < parsedArgs->recordNum && process; i++) {

        pthread_mutex_lock(&mutex);
        if((size = read(fileDesc, record, RECORD_SIZE)) < 0) {
            pthread_mutex_unlock(&mutex);
            perror("Reading error");
            exit(EXIT_FAILURE);
        }

        if(size == 0)
            process = 0;

        pthread_mutex_unlock(&mutex);
        pthread_testcancel();

        if (size > 0) {
            char *delim = strchr(record, ';');
            record[delim - record] = '\0';
            int id = atoi(record);
            record += delim - record + 1; // skip id and delimiter

            printf("[Test] id: %d, text: %s\n", id, record);

            char *wordPtr = strstr(record, parsedArgs->word);
            if (wordPtr != NULL) {
                return id;
            }
        }
    }
    return NOT_FOUND;
}

void cleanup(void *buf) {
    free(buf);
}

void make_threads() {
   for (int i=0; i<parsedArgs->threadNum; i++) {
       if (pthread_create(&pthreads[i], NULL, parse, NULL) == -1) {
           perror("creating thread error");
           exit(EXIT_FAILURE);
       }
   }

   for (int i=0; i<parsedArgs->threadNum; i++) {
       pthread_join(pthreads[i], NULL);
   }
}

void *parse(void *args) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    int recId = find_word();

    if (recId != NOT_FOUND) {
        for (int i=0; i<parsedArgs->threadNum; i++) {
            if (pthreads[i] != pthread_self())
                pthread_cancel(pthreads[i]);
        }
        printf("TID: %ld, record ID: %d\n", pthread_self(), recId);
    }
    pthread_exit(0);
}

void init() {
    pthreads = (pthread_t *) calloc((size_t) parsedArgs->recordNum, sizeof(pthread_t));
    pthread_key_create(&thread_log_key, cleanup);

    fileDesc = open(parsedArgs->fileName, O_RDONLY);
    if (fileDesc == -1) {
        perror("opening file error");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&mutex, NULL) < 0) {
        perror("pthread_mutex_init error");
        exit(EXIT_FAILURE);
    }
}

void parse_args(int argc, char **argv) {
    if (argc < 5) {
        printf("To less arguments. Syntax: <thread_num> <data_file_name> <record_num> <search_word>\n");
        exit(EXIT_FAILURE);
    }
    parsedArgs->exePath = argv[0];
    parsedArgs->threadNum = atoi(argv[1]);
    parsedArgs->fileName = argv[2];
    parsedArgs->recordNum = atoi(argv[3]);
    parsedArgs->word = argv[4];
}
