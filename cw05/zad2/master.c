#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

#define RE_MAX 1
#define RE_MIN -2
#define IM_MAX 1
#define IM_MIN -1

typedef struct ParsedArgs {
    char *exePath,
            *path; // path to fifo
            int R; // size of array (T) which represents screen (pixels)
} ParsedArgs;

// globals
int slavesAreReady = 0;

void parse_args(int, char **, ParsedArgs *);
void config();
void sig_handler(int, siginfo_t *, void *);
void make_fifo(char *);
int ** read_from_fifo(char *, int);
void save_data_to_file(int **, int);
void open_gnuplot(int);

int main(int argc, char ** argv) {
    config();
    ParsedArgs *parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv, parsedArgs);
    make_fifo(parsedArgs->path);

    printf("Generating 'data.txt' file with data to plot.\n");
    int **T = read_from_fifo(parsedArgs->path, parsedArgs->R);
    save_data_to_file(T, parsedArgs->R);
    printf("File 'data.txt' has been generated successfully.\n");

    // clean
    for (int i=0; i<parsedArgs->R; i++) free(T[i]);
    free(T);
    unlink(parsedArgs->path); // remove named pipe

    printf("Opening 'data.txt' in gnuplot...\n");
    printf("Enter any key to exit.\n");
    open_gnuplot(parsedArgs->R);


    free(parsedArgs);
    return 0;
}

void parse_args(int argc, char **argv, ParsedArgs *parsedArgs) {
    if (argc < 3) { printf("To less arguments\n"); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->path = argv[1];
    parsedArgs->R = (int) strtol(argv[2], NULL, 10);
}

void config() {
    struct sigaction act;
    act.sa_sigaction = &sig_handler;
    act.sa_flags = SA_SIGINFO;

    sigaction(SIGRTMIN+1, &act, NULL); // slaves are ready
}

void sig_handler(int sig, siginfo_t *siginfo, void *context) {
    if (sig == SIGRTMIN + 1) {
        //printf("Master: Slaves are ready\n");
        // slaves are ready, master can start reading form fifo
        slavesAreReady = 1;
    }
}

void make_fifo(char *path) {
    if(mkfifo(path, S_IRUSR | S_IWUSR) == -1) {
        printf("Error while creating fifo by master\n");
        kill(getppid(), SIGRTMIN+2);
        exit(1);
    }
    kill(getppid(), SIGRTMIN);
}

int ** read_from_fifo(char *path, int R) {
    FILE* fp = fopen(path, "r");
    char buff[100];
    double a, b;
    int iters;

    if(fp == NULL){
        printf("Error while opening fifo by master!\n");
        kill(getppid(), SIGRTMIN+2);
        exit(1);
    }

    while(slavesAreReady != 1){ sleep(1); }

    // make table for "pixels"
    int **T = malloc(sizeof(int *) * R);
    for (int i=0; i<R; i++) {
        T[i] = malloc(sizeof(int) * R);
    }

    // read from named pipe
    while (fgets(buff, 100, fp) != NULL) {
        sscanf(buff, "%lf %lf %d\n", &a, &b, &iters);
        int i = (int) ((R-1) * (a - RE_MIN) / (RE_MAX - RE_MIN));
        int j = (int) ((R-1) * (b - IM_MIN) / (IM_MAX - IM_MIN));
        T[i][j] = iters;
    }

    fclose(fp);

    return T;
}

void save_data_to_file(int **T, int R) {
    FILE *fp = fopen("data.txt", "w+");
    for(int i=0; i<R; i++){
        for(int j=0; j<R; j++){
            fprintf(fp, "%d %d %d\n", i, j, T[i][j]);
        }
    }
    fclose(fp);
}

void open_gnuplot(int X) {
    FILE *gnup = popen("gnuplot", "w");
    fprintf(gnup, "set view map\n");
    fprintf(gnup, "set xrange [0:%d]\n", X);
    fprintf(gnup, "set yrange [0:%d]\n", X);
    fprintf(gnup, "plot 'data.txt' with image\n");

    fflush(gnup);
    getc(stdin);
    pclose(gnup);
}