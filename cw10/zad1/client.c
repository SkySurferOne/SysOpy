#include "common.h"

// structs
typedef struct ParsedArgs {
    char *exePath;
    int use_inet;
    char name[MAX_NAME_LEN];
    char path[PATH_MAX];
    char *ip;
    in_port_t port;
} ParsedArgs;

// globals
int soc;
ParsedArgs *parsed_args;

// api
void init_inet_soc();
void init_unix_soc();
void parse_args(int, char **);
void loop();
void help();
void init();
void send_greeting();
void clear();
void perform_task(int, int, int);
void sigint_handler(int);

int main(int argc, char *argv[]) {
    parsed_args = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    init();
    loop();

    return 0;
}

void loop() {
    if (parsed_args->use_inet) {
        init_inet_soc();
    } else {
        init_unix_soc();
    }

    send_greeting();

    while (1) {
        message msg;
        ssize_t count = recv(soc, &msg, sizeof(msg), MSG_WAITALL);

        if (count == -1) {
            if (errno != EAGAIN) {
                perror("recv");
                exit(EXIT_FAILURE);
            }
        }
        else if (count == 0) {
            exit(EXIT_SUCCESS);
        }
        else if (count < sizeof(msg)) {
            continue;
        }

        switch (msg.msg_type) {
            case PERFORM_TASK:
                printf("Received a task\n");
                fflush(stdout);
                perform_task(msg.task, msg.id, msg.msg_type);
                break;
            case PING:
                printf("Pinged\n");
                fflush(stdout);
                // send_alive();
                break;
            case KILL:
                exit(EXIT_SUCCESS);
            default:
                printf("unknown command\n");
        }
    }
}

void perform_task(int task, int id, int task_type) {
    message msg;
    msg.msg_type = RESULT;

    int result = 0;
    msg.id = id;

    switch (task_type) {
        case SUM:
            result = msg.a + msg.b;
            break;
        case SUB:
            result = msg.a - msg.b;
            break;
        case MUL:
            result = msg.a * msg.b;
            break;
        case DIV:
            result = msg.a / msg.b;
            break;
        default:
            printf("unknown task\n");
    }

    msg.result = result;

    if (send(soc, &msg, sizeof(msg), 0) == -1) {
        perror("send error");
    }
}

void init() {
    atexit(clear);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
}

void clear() {
    if (soc != -1) {
        if (shutdown(soc, SHUT_RDWR) == -1) {
            perror("socket shutdown error");
        }

        if (close(soc) == -1) {
            perror("closing socket error");
        }

        soc = -1;
    }

    free(parsed_args);
}

void sigint_handler(int sig) {
    clear();
    exit(EXIT_SUCCESS);
}

void help() {
    printf("Too less arguments. Write:\n"
                   "name \n"
                   "\t1 - inet / 0 - unix\n"
                   "\tIP / path\n"
                   "\tport - in inet case\n");
}

void parse_args(int argc, char **argv) {
    parsed_args->exePath = argv[0];
    if (argc < 4) {
        help();
        exit(EXIT_FAILURE);
    }

    strncpy(parsed_args->name, argv[1], MAX_NAME_LEN);
    parsed_args->name[MAX_NAME_LEN - 1] = '\0';
    parsed_args->use_inet = atoi(argv[2]);
    if (parsed_args->use_inet != 0 && parsed_args->use_inet != 1) {
        perror("Wrong parameters");
        help();
        exit(EXIT_FAILURE);
    }

    if (parsed_args->use_inet) {
        parsed_args->ip = argv[3];
        parsed_args->port = (in_port_t) atoi(argv[4]);
    }
    else {
        strncpy(parsed_args->path, argv[3], PATH_MAX);
        parsed_args->path[PATH_MAX - 1] = '\0';
    }
}

void init_inet_soc() {
    struct sockaddr_in sockaddr_inet_serv;

    soc = socket(AF_INET, SOCK_STREAM, 0);
    if (soc == -1) {
        perror("socket error - inet");
        exit(EXIT_FAILURE);
    }

    memset(&sockaddr_inet_serv, 0, sizeof(sockaddr_inet_serv));
    sockaddr_inet_serv.sin_family = AF_INET;
    sockaddr_inet_serv.sin_port = htons(parsed_args->port);

    if (inet_aton(parsed_args->ip, &sockaddr_inet_serv.sin_addr) == 0) {
        perror("inet_aton error");
        exit(EXIT_FAILURE);
    }

    if (connect(soc, (const struct sockaddr *) &sockaddr_inet_serv, sizeof(sockaddr_inet_serv)) == -1) {
        perror("connect error - inet");
        exit(EXIT_FAILURE);
    }
}

void init_unix_soc() {
    soc = socket(AF_UNIX, SOCK_STREAM, 0);
    if (soc == -1) {
        perror("socket error - unix");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un sockaddr_un_serv;
    memset(&sockaddr_un_serv, 0, sizeof(sockaddr_un_serv));
    sockaddr_un_serv.sun_family = AF_UNIX;

    strncpy(sockaddr_un_serv.sun_path, parsed_args->path, PATH_MAX);

    printf("%s\n", sockaddr_un_serv.sun_path);

    if (connect(soc, (const struct sockaddr *) &sockaddr_un_serv, sizeof(sockaddr_un_serv)) == -1) {
        perror("connect error - unix");
        exit(EXIT_FAILURE);
    }
}

void send_greeting() {
    message msg;
    msg.msg_type = CONNECT;
    strncpy(msg.name, parsed_args->name, MAX_NAME_LEN);

    if (send(soc, &msg, sizeof(msg), 0) == -1) {
        perror("send error");
    }
}