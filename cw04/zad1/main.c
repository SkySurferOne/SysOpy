#include<stdio.h>
#include<stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


int reverse = 0;

void delay(int);
void print_letters();
void config();
void toggle_reverse(int);
void handle_sigint(int, siginfo_t *, void *);

int main() {
    config();
    print_letters();

    return 0;
}

void toggle_reverse(int sig) {
    reverse = (reverse == 0) ? 1 : 0;
}

void handle_sigint(int sig, siginfo_t *siginfo, void *context) {
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

void config() {
    printf("%d\n", getpid());
    signal(SIGTSTP, toggle_reverse);

    struct sigaction act;
    act.sa_sigaction = &handle_sigint;
    act.sa_flags = SA_SIGINFO;

    sigaction(2, &act, NULL);
}

void print_letters() {
    char c = (reverse == 0) ? 'A' : 'Z';

    while (1) {
        fflush(stdout);
        printf("%c\n", c);

        if (!reverse)
            c += 1;
        else
            c -= 1;
        if (c > 'Z') c = 'A';
        if (c < 'A') c = 'Z';

        delay(500);
    }
}

void delay(int milliseconds) {
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}