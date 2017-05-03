#include "common.h"

// globals
int clientId = UNREGISTERED;
mqd_t serverQueue;
mqd_t clientQueue;
pid_t globalppid = -1;
char name[10];

// api
void set_server_queue();
mqd_t make_queue();
void clean();
void make_name();
void register_on_server(mqd_t);
void send_msg(int req, char* buf);
void show_help();
void listen_res();
void open_req_loop();

int main() {
    pid_t pid;
    globalppid = getpid();
    printf(ANSI_COLOR_YELLOW "[DEBUG] client PID: %d" ANSI_COLOR_RESET "\n", globalppid);

    make_name();

    set_server_queue();
    clientQueue = make_queue();
    register_on_server(clientQueue);

    pid = fork();

    if(pid == 0)
        listen_res();
    else
        open_req_loop();

    return 0;
}

void make_name() {
    name[0] = '/';
    sprintf(name+1, "%d", globalppid);
    printf("Queue name: '%s'\n", name);
}

void set_server_queue() {
    if((serverQueue = mq_open(SERVER_NAME, O_RDWR)) == -1) {
        printf("Cannot open server queue\n");
        exit(EXIT_FAILURE);
    }
}

mqd_t make_queue() {
    struct mq_attr attr;

    attr.mq_flags = MQ_FLAGS;
    attr.mq_maxmsg = MQ_MAXMSG;
    attr.mq_msgsize = MQ_MSGSIZE;
    attr.mq_curmsgs = MQ_CURMSG;

    mqd_t queue = mq_open(name, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &attr);

    if(queue < 0){
        printf("mq_open error: cannot make queue %s\n", name);
        exit(EXIT_FAILURE);
    }

    return queue;
}

void register_on_server(mqd_t queue) {
    char buf[MQ_CURMSG];
    buf[0] = REGISTER_REQ;
    strcpy(buf + 1, name);

    if(mq_send(serverQueue, buf, MQ_MSGSIZE, 0) == -1){
        printf("mq_send error: cannot send a message\n");
        exit(EXIT_FAILURE);
    }

    unsigned int priop;
    if(mq_receive(clientQueue, buf, MQ_MSGSIZE, &priop) == -1) {
        printf("mq_send error: cannot receive a message\n");
        exit(EXIT_FAILURE);
    }

    if(buf[0] == REGISTER_FAIL){
        printf("Cannot register\n");
        exit(EXIT_FAILURE);
    }
    clientId = (int) buf[1];
    printf("Registered with id: %d\n", clientId);
}

void open_req_loop() {
    char cmd;
    char buf [MQ_CURMSG];
    char bufInput[MQ_CURMSG];

    printf("Write command. Write 'h' if you need help.\n");
    while(1) {
        buf[1] = (char) clientId;
        buf[2] = 0;

        printf("> ");
        scanf(" %c", &cmd);
        getchar();

        if(cmd == 'e' || cmd == 'u') {
            fflush(stdin);
            fgets(bufInput, sizeof(bufInput), stdin);
            int ind = (int) strlen(bufInput)-1;
            if(bufInput[ind] == '\n') bufInput[ind] = '\0';
            strcpy(buf + 2, bufInput);

        }

        switch(cmd) {
            case 'e':
                send_msg(ECHO_REQ, buf);
                break;
            case 'u':
                send_msg(UPPERCASE_REQ, buf);
                break;
            case 't':
                send_msg(TIME_REQ, buf);
                break;
            case 's':
                send_msg(TERMINATE_REQ, buf);
                break;
            case 'q':
                send_msg(LOGOUT_REQ, buf);
                break;
            case 'h':
                show_help();
                break;
            default:
                printf("Unknown command. Press write 'h' to see help.\n");
        }
        if(cmd == 'q' || cmd == 's') break;
        fflush(stdin);
    }
    while(1){}; // wait for kill
}

void send_msg(int req, char* buf) {
    buf[0] = (char) req;

    if(mq_send(serverQueue, buf, MQ_MSGSIZE, 0) == -1){
        perror("mq_send: sending message failed");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Message: \"%s\" has been sent\n", buf);
    }
}

void listen_res() {
    char buf[MQ_MSGSIZE];
    unsigned int priop;

    while(1){
        ssize_t receive_size = mq_receive(clientQueue, buf, MQ_MSGSIZE, &priop);

        if(receive_size > 0){
            if (buf[0] == PRINT) {
                printf("Response: %s\n", buf + 2);
            } else {
                clean();
            }
        }
    }
}

void clean() {
    mq_close(serverQueue);
    mq_close(clientQueue);
    mq_unlink(name);
    kill(getppid(), SIGKILL);
    exit(EXIT_SUCCESS);
}

void show_help() {
    printf("\tCommand help:\n"
                   "\t\te - echo request\n"
                   "\t\tu - uppercase request\n"
                   "\t\tt - time request\n"
                   "\t\ts - stop server request\n"
                   "\t\tq - quit and logout\n"
                   "\t\th - show help\n\n");
}