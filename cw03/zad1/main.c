#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>

#define LINE_MAX_LEN 1024
#define MAX_ARGS_NUM 20
#define INSTR_MAX_LEN 255

typedef struct ParsedArgs {
    char *exePath;
    char *filePath;
} ParsedArgs;

void parse_args(int, char **, ParsedArgs *);
void parse_line(char *);
void parse_file(char *);
void eval_envvar_cmd(char *);
void eval_cmd(char *);
char * resolve_arg(char *);

int main(int argc, char ** argv) {
    ParsedArgs *parsedArgs = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv, parsedArgs);
    parse_file(parsedArgs->filePath);
    free(parsedArgs);
    return 0;
}

void parse_file(char *filePath) {
    FILE *fptr;
    fptr = fopen(filePath, "r");
    if (fptr == NULL) {
        printf("Cannot file the file %s\n", filePath);
        exit(1);
    }

    char buffer[LINE_MAX_LEN];
    while(fgets(buffer, LINE_MAX_LEN, fptr)) {
        parse_line(buffer);
    }

    fclose(fptr);
}

void parse_line(char *line) {
    int i = 0;
    while (line[i] == ' ') i++;
    char ch = line[i];

    switch (ch) {
        case '#':
            eval_envvar_cmd(line+i+1);
            break;
        case '\n':
        case '\0':
            break;
        default:
            eval_cmd(line+i);
    }
}

void eval_envvar_cmd(char *cmd) {
    char varName[INSTR_MAX_LEN];
    char varValue[INSTR_MAX_LEN];
    int i=0;

    for (i=0; cmd[i] != ' ' && cmd[i] != '\n' && cmd[i] != '\0'; i++);
    if (i == 0) {
        printf("Invalid envvar cmd\n");
        return;
    }
    if (i >= INSTR_MAX_LEN) { printf("Instruction is too long\n"); exit(1); }
    strncpy(varName, cmd, (size_t) i);
    varName[i] = '\0';

    if (cmd[i] == '\n' || cmd[i] == '\0') { // unset envvar
        if(unsetenv(varName) == -1) {
            printf("Sth went wrong in unsetenv\n"); exit(1);
        }

    } else {
        while (cmd[i] == ' ') i++;
        if (cmd[i] == '\n' || cmd[i] == '\0') return; // just a couple spaces more
        cmd += i; i=0;
        for (;cmd[i] != ' ' && cmd[i] != '\n' && cmd[i] != '\0'; i++);
        if (i >= INSTR_MAX_LEN) { printf("Instruction is too long\n"); exit(1); }
        strncpy(varValue, cmd, (size_t) i);
        varValue[i] = '\0';

        if(setenv(varName, varValue, 1) == -1) {
            printf("Something went wrong in setenv\n"); exit(1);
        }
    }
}

char * resolve_arg(char * arg) {
    if (arg == NULL) return "";
    if (arg[0] == '$') {
        char *s = getenv(arg+1);
        if (!s) { printf("Env variable %s doesn't exists\n", arg+1); exit(1); }
        return s;
    }

    return arg;
}

void eval_cmd(char *cmd) {
    char cmdName[INSTR_MAX_LEN];
    char *cmdArgs[MAX_ARGS_NUM + 2];
    int i=0, count;

    for (i=0; cmd[i] != ' ' && cmd[i] != '\n' && cmd[i] != '\0'; i++);
    if (i == 0) { printf("Invalid cmd\n"); return; }
    if (i >= 255) { printf("Instruction is too long\n"); exit(1); }
    strncpy(cmdName, cmd, (size_t) i);
    cmdName[i] = '\0';

    cmdArgs[0] = cmdName;
    while (cmd[i] == ' ') i++;
    if (cmd[i] == '\n' || cmd[i] == '\0') { // no args
        cmdArgs[1] = (char *) NULL;
    } else {
        cmd += i; i=0;
        count = 1;
        for(; cmd[i] != '\n' && cmd[i] != '\0'; i++) {
            if (cmd[i] == ' ') {
                cmd[i] = '\0';
                if (count + 1 < MAX_ARGS_NUM + 1) {
                    cmdArgs[count++] = resolve_arg(cmd);

                } else { printf("To much arguments\n"); exit(1); }
                i++;
                while (cmd[i] == ' ') i++;
                cmd +=i; i=0;
            }
        }
        cmd[i] = '\0';
        if (count + 1 < MAX_ARGS_NUM + 1) cmdArgs[count++] = resolve_arg(cmd); else { printf("To much arguments\n"); exit(1); }
        cmdArgs[count] = (char *) NULL;
    }

    int pid = fork();
    if (pid == -1) {
        printf("%s\n", "Failed to fork"); exit(1);
    } else if (pid == 0) {
        int res = execvp(cmdName, cmdArgs);
        if(res == -1) abort(); else exit(res);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (status != 0) {
            printf("Command termination failed - Error %d", status);
            exit(1);
        }
    }
}

void parse_args(int argc, char ** argv, ParsedArgs *parsedArgs) {
    if (argc < 2) { printf("To less arguments\n"); exit(1); }
    parsedArgs->exePath = argv[0];
    parsedArgs->filePath = argv[1];
}