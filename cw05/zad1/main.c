#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define LINE_MAX_LEN 1024
#define MAX_ARGS_PER_CMD 3
#define MAX_CMDS 20

void get_lines();
void config();
void quit_program(int, siginfo_t *, void *);
void parse_line(char *);
void eval_cmd(char *);

int main() {
    config();
    printf("Print pipe chain for ex.: 'ps aux | grep root | head -10'. \nTo quit enter 'q'.\n");
    get_lines();
    return 0;
}

void config() {
    struct sigaction act;
    act.sa_sigaction = &quit_program;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &act, NULL);
}

void quit_program(int sig, siginfo_t *siginfo, void *context) {
    printf("Kill proc\n");
    // TODO kill children
}

void parse_line(char * line) {
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    char ch = line[i];

    switch (ch) {
        case '\n':
        case '\0':
            break;
        default:
            eval_cmd(line + i);
    }
}

void eval_cmd(char *cmd) {
    char cmdName[LINE_MAX_LEN];
    char *cmdArgs[MAX_ARGS_PER_CMD + 2];
    int fd[2], i = 0, count = 0, process = 1, cmds_no = 0, in_desc;
    pid_t w;

    in_desc = STDIN_FILENO;

    while(process == 1) { // process piped command
        if (pipe(fd) == -1) { // open one-way communication channel
            printf("Pipe error\n");
            exit(1);
        }

        cmds_no++;
        if (cmds_no >= MAX_CMDS) { printf("To much commands. Limit is 20.\n"); exit(1); }

        // ========================================= parse command
        i = 0;
        while (cmd[i] == ' ' || cmd[i] == '\t') i++;
        cmd += i;

        for (i = 0; cmd[i] != ' ' && cmd[i] != '\t' && cmd[i] != '\n' && cmd[i] != '\0' && cmd[i] != '|'; i++);
        if (i == 0) {
            printf("Invalid cmd\n");
            return;
        }
        if (i >= LINE_MAX_LEN) {
            printf("Instruction is too long\n");
            exit(1);
        }
        strncpy(cmdName, cmd, (size_t) i);
        cmdName[i] = '\0';

        cmdArgs[0] = cmdName;
        while (cmd[i] == ' ' || cmd[i] == '\t') i++;
        if (cmd[i] == '\n' || cmd[i] == '\0' || cmd[i] == '|') { // no args
            cmdArgs[1] = (char *) NULL;
            if (cmd[i] == '\n') {
                process = 0;
            }
            cmd[i] = '\0';
            cmd += i + 1;
        } else {
            cmd += i;
            i = 0;
            count = 1;
            while (cmd[i] != '\n' && cmd[i] != '\0' && cmd[i] != '|') {
                i++;
                if (cmd[i] == ' ') {
                    cmd[i] = '\0';
                    if (count + 1 < MAX_ARGS_PER_CMD + 1) {
                        cmdArgs[count++] = cmd;
                    } else {
                        printf("To much arguments\n"); exit(1);
                    }
                    i++;
                    while (cmd[i] == ' ' || cmd[i] == '\t') i++;
                    cmd += i; i = 0;
                }
            }
            if (cmd[i] == '\n') {
                process = 0;
            }
            cmd[i] = '\0';
            if (*cmd != '\0') {
                if (count + 1 < MAX_ARGS_PER_CMD + 1) cmdArgs[count++] = cmd;
                else {
                    printf("To much arguments\n");exit(1);
                }
            }
            cmdArgs[count] = (char *) NULL;
            cmd += i + 1;
        }
        // ========================================= parse command end

        // execute command from chain
        if (process == 1) {
            int pid = fork();

            if (pid < 0) {
               printf("Fork error\n");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                if (in_desc != STDIN_FILENO) {
                    if (dup2(in_desc, STDIN_FILENO) == -1)
                        printf("dup2 error to stdin\n");
                    close(in_desc);
                }

                if (fd[1] != STDOUT_FILENO) {
                    if (dup2(fd[1], STDOUT_FILENO) == -1) {
                        printf("dup2 error to stdout\n");
                    }
                    close(fd[1]);
                }

                int res = execvp(cmdName, cmdArgs);
                if(res == -1) abort(); else exit(res);

            } else {
                int status;
                w = waitpid(pid, &status, 0);
                if (w == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }

                if (status != 0) {
                    printf("Command termination failed - Error %d", status);
                    exit(1);
                }

            }

        } else {
            int pid = fork();

            if (pid < 0) {
                printf("Fork error\n");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                if (in_desc != STDIN_FILENO) {
                    if (dup2(in_desc, STDIN_FILENO) == -1)
                        printf("dup2 error to stdin\n");
                    close(in_desc);
                }

                int res = execvp(cmdName, cmdArgs);
                if(res == -1) abort(); else exit(res);

            } else {
                int status;
                w = waitpid(pid, &status, 0);
                if (w == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }

                if (status != 0) {
                    printf("Command termination failed - Error %d", status);
                    exit(1);
                }
            }

        }

        close(fd[1]);
        in_desc = fd[0];

    }
}

void get_lines() {
    while (1) {
        char str[LINE_MAX_LEN];
        printf("Enter a value : ");
        fgets(str, sizeof(str), stdin);
        if (*str == 'q') break;
        parse_line(str);
        fflush(stdin);
    }
}
