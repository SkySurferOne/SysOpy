#include "common.h"

typedef struct client {
    mqd_t queue;
    int id;
} client;

// globals
client clients[CLIENTS_MAX_NUM];
int clientCounter = 0;
int idCounter = 1;
mqd_t serverQueue;

// api
mqd_t make_queue(char* name);
void clean();

void register_client(char *);
void echo_service(char*);
void uppercase_service(char*);
void time_service(char*);
void terminate(char*);
void client_logout(char*);

void get_msg_from_clients();

void send_msg(int, int, char*);
void show_clients();
void request_handler(char*);


int main() {
    printf("Server is running...\n");
    serverQueue = make_queue(SERVER_NAME);
    printf("Waiting for messages from clients.\n");
    get_msg_from_clients();

    return 0;
}

mqd_t make_queue(char* name) {
    struct mq_attr attr;
    mode_t omask;

    attr.mq_flags = MQ_FLAGS;
    attr.mq_maxmsg = MQ_MAXMSG;
    attr.mq_msgsize = MQ_MSGSIZE;
    attr.mq_curmsgs = MQ_CURMSG;

    omask = umask(0);
    mqd_t queue = mq_open(name, O_RDONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    umask(omask);

    if(queue < 0){
        printf("mq_open error: cannot make queue %s\n", name);
        exit(EXIT_FAILURE);
    }

    return queue;
}

void get_msg_from_clients() {
    unsigned int priop;
    char buf[MQ_MSGSIZE];

    while(1) {
        ssize_t recSize;
        recSize = mq_receive(serverQueue, buf, MQ_MSGSIZE, &priop);

        if(recSize > 0) {
            printf(ANSI_COLOR_YELLOW "[DEBUG] Received message: '%s' with type %d" ANSI_COLOR_RESET "\n", buf, buf[0]);
            request_handler(buf);
        }
    }
}

void request_handler(char* buf) {
    switch ((int) buf[0]) {
        case REGISTER_REQ:
            register_client(buf);
            break;
        case ECHO_REQ:
            echo_service(buf);
            break;
        case UPPERCASE_REQ:
            uppercase_service(buf);
            break;
        case TIME_REQ:
            time_service(buf);
            break;
        case TERMINATE_REQ:
            terminate(buf);
            break;
        case LOGOUT_REQ:
            client_logout(buf);
            break;
        default:
            printf("Unknown message type\n");
    }
}

void register_client(char* buf) {
    mqd_t queue;

    if (clientCounter >= CLIENTS_MAX_NUM) {
        perror("Cannot add new client. Limit has expired.\n");
        buf[0] = REGISTER_FAIL;
    } else {
        buf[0] = REGISTER_OK;
    }

    if ((queue = mq_open(buf+1, O_RDWR)) == -1) {
        printf("mq_open error: cannot open client queue with the name '%s'\n", buf+1);
        return;
    }

    buf[1] = (char) idCounter;
    buf[2] = 0;

    if(mq_send(queue, buf, MQ_MSGSIZE, 0) == -1){
        printf("mq_send error: cannot send a message\n");
        return;
    }
    if(buf[0] != REGISTER_FAIL) {
        client clientStr;
        clientStr.id = idCounter++;
        clientStr.queue = queue;
        clients[clientCounter] = clientStr;

        clientCounter++;
        printf("Client no %d has been registered\n", clientCounter);
        show_clients();
    }
}

mqd_t get_client_mqd_t_by_id(int id) {
    int i;
    mqd_t queue = -1;

    for (i=0; i<clientCounter; i++) {
        if (clients[i].id == id) {
            queue = clients[i].queue;
            break;
        }
    }

    if (queue != -1) {
        return queue;
    } else {
        return -1;
    }
}

void send_msg(int id, int req, char* buf) {
    mqd_t queue;
    buf[0] = (char) req;

    if((queue = get_client_mqd_t_by_id(id)) == -1) {
        printf("Cannot open client queue with id %d\n", id);
        return;
    }

    if(mq_send(queue, buf, MQ_MSGSIZE, 0) == -1){
        printf("mq_send error: cannot send a message\n");
        return;
    }
}

void echo_service(char* buf) {
    send_msg((int) buf[1], PRINT, buf);
}

void uppercase_service(char* buf) {
    for (int i=2; buf[i] != '\0'; i++) {
        char c = buf[i];
        if (c >= 97 && c <= 122)
            buf[i] -= 32;
    }
    echo_service(buf);
}

void time_service(char* buf) {
    time_t timeval;
    time(&timeval);
    struct tm * localtimeval = localtime(&timeval);
    strcpy(buf+2, asctime(localtimeval));
    int ind = (int) strlen(buf)-1;
    if (buf[ind] == '\n') buf[ind] = '\0';
    echo_service(buf);
}

void terminate(char* buf) {
    for (int i=0; i<clientCounter; i++) {
        send_msg(clients[i].id, TERMINATE_REQ, buf);
    }
    printf("Server termination\n");
    clean();
}

void client_logout(char* buf) {
    int ind = 0, change = 0;
    int id = (int) buf[1];
    send_msg(id, TERMINATE_REQ, buf);

    for (int i=0; i<clientCounter-1; i++) {
        if(clients[i].id != id) ind++;
        else change = 1;
        if(change == 1) clients[ind] = clients[i+1];
    }

    clientCounter--;
    show_clients();
}

void clean() {
    for(int i = 0; i <clientCounter; i++){
        if(clients[i].queue != -1){
            mq_close(clients[i].queue);
        }
    }
    mq_close(serverQueue);
    mq_unlink(SERVER_NAME);
    exit(EXIT_SUCCESS);
}

void show_clients() {
    printf(ANSI_COLOR_MAGENTA "Clients number: %d" ANSI_COLOR_RESET "\n", clientCounter);
    for (int i=0; i<clientCounter; i++) {
        printf(ANSI_COLOR_MAGENTA "\tno.: %d, id: %d, mqd_t: %d" ANSI_COLOR_RESET "\n", i, clients[i].id, clients[i].queue);
    }
}