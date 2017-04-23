#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


#define RE_MAX 1
#define RE_MIN -2
#define IM_MAX 1
#define IM_MIN -1

typedef struct ParsedArgs {
    char *exePath,
            *path; // path to fifo
           long N; // number of points
            int K; // number of iterations

} ParsedArgs;

void parse_args(int, char **, ParsedArgs *);
void write_to_fifo(char *, long, int);
double complex get_rand_complex_point();
int get_iters(double complex, int);

int main(int argc, char ** argv) {
    srand(time(NULL));
    ParsedArgs *parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv, parsedArgs);
    write_to_fifo(parsedArgs->path, parsedArgs->N, parsedArgs->K);

    free(parsedArgs);
    return 0;
}

void parse_args(int argc, char **argv, ParsedArgs *parsedArgs) {
    if (argc < 4) { printf("To less arguments\n"); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->path = argv[1];
    parsedArgs->N = strtol(argv[2], NULL, 10);
    parsedArgs->K = (int) strtol(argv[3], NULL, 10);
}

void write_to_fifo(char *path, long N, int K) {
    FILE *fp = fopen(path, "a");
    if(fp == NULL){
        printf("Error while opening fifo by master\n");
        kill(getppid(), SIGRTMIN+2);
        exit(1);
    }
    kill(getppid(), SIGRTMIN + 1); // send to ppid info that im ready

    for (int i=0; i<N; i++) {
        double complex c = get_rand_complex_point();
        int iters = get_iters(c, K);
        fprintf(fp, "%.3f %.3f %d\n", creal(c), cimag(c), iters);
    }
    fclose(fp);
}

double complex get_rand_complex_point() {
    double a = ((double) rand()/RAND_MAX) * (RE_MAX - RE_MIN) + RE_MIN;
    double b = ((double) rand()/RAND_MAX) * (IM_MAX - IM_MIN) + IM_MIN;
    return a + b * I;
}

int get_iters(double complex c, int K) {
    double complex zn = c;
    int k = 0;
    while (cabs(zn) <= 2.0 && k < K) {
        k++;
        zn = cpow(zn, 2.0) + c;
    }
    return k;
}