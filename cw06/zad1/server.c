#include "common.h"

#define CLIENTS_MAX_NUM 20

typedef struct client {
    key_t key;
    pid_t pid;
} client;

// globals
client clients[CLIENTS_MAX_NUM];
int clientCounter = 0;
int idCounter = 0;
int serverMsqid;

// api
int make_queue();
void get_msg_from_clients(int);
void remove_queue();
void request_handler(message_buf);
void register_client(char *, pid_t);
void echo_service(message_buf);
void uppercase_service(message_buf);
void time_service(message_buf);
void terminate();
void client_logout(message_buf);
int get_client_msqid_by_pid(pid_t);
void send_msg(int, char*, pid_t);
void show_clients();

// ipcrm --all=msg
int main() {
    printf("Server is running...\n");
    serverMsqid = make_queue();
    printf("Waiting for messages from clients.\n");
    get_msg_from_clients(serverMsqid);

    return 0;
}

int make_queue() {
    key_t key;
    int msgflg, msqid;

    msgflg = IPC_CREAT | 0666;

    if ((key = ftok(getenv("HOME"), SERVER_KEY)) == -1) {
        perror("ftok: generating IPC key failed");
        exit(EXIT_FAILURE);
    }

    if((msqid = msgget(key, msgflg)) == -1) {
        perror("msgget: making queue failed");
        exit(EXIT_FAILURE);
    }

    printf(ANSI_COLOR_YELLOW "[DEBUG] server_key %d, server_msqid: %d" ANSI_COLOR_RESET "\n", key, msqid);
    return msqid;
}

void get_msg_from_clients(int msqid) {
    message_buf  rbuf;
    struct msqid_ds buf;

    while(1) {
        msgctl(msqid, IPC_STAT, &buf);

        if(buf.msg_qnum > 0) {

            if (msgrcv(msqid, &rbuf, MSGBUF_SIZE, 0, IPC_NOWAIT) < 0) {
                perror("msgrcv: receiving message failed");
                exit(EXIT_FAILURE);
            }

            printf(ANSI_COLOR_YELLOW "[DEBUG] Received message: %s with type %ld" ANSI_COLOR_RESET "\n", rbuf.mtext, rbuf.mtype);
            request_handler(rbuf);
        }
    }
}

void request_handler(message_buf rbuf) {
    switch (rbuf.mtype) {
        case REGISTER_REQ:
            register_client(rbuf.mtext, rbuf.senderPid);
            break;
        case ECHO_REQ:
            echo_service(rbuf);
            break;
        case UPPERCASE_REQ:
            uppercase_service(rbuf);
            break;
        case TIME_REQ:
            time_service(rbuf);
            break;
        case TERMINATE_REQ:
            terminate();
            break;
        case LOGOUT_REQ:
            client_logout(rbuf);
            break;
        default:
            printf("Unknown message type\n");
    }
}

void echo_service(message_buf buf) {
    int msqid;

    if((msqid = get_client_msqid_by_pid(buf.senderPid)) < 0) {
        printf("Cannot open queue for PID %d\n", buf.senderPid);
        return;
    }

    message_buf newMsg;
    newMsg.senderPid = getpid();
    sprintf(newMsg.mtext, "%s", buf.mtext);
    newMsg.mtype = PRINT;

    if (msgsnd(msqid, &newMsg, MSGBUF_SIZE, 0) < 0) {
        printf("Cannot send message to %d msqid\n", msqid);
        return;
    }
}

int get_client_msqid_by_pid(pid_t pid) {
    int key = -1, msqid = -1, i;

    for (i=0; i<clientCounter; i++) {
        if (clients[i].pid == pid) {
            key = clients[i].key;
            break;
        }
    }

    if (i != clientCounter) {
        if ((msqid = msgget(key, 0666)) < 0) {
            perror("msgget: Opening server queue failed");
            return -1;
        }
        return msqid;
    } else {
        return -1;
    }
}

void uppercase_service(message_buf buf) {
    for (int i=0; buf.mtext[i] != '\0'; i++) {
        char c = buf.mtext[i];
        if (c >= 97 && c <= 122)
            buf.mtext[i] -= 32;
    }
    echo_service(buf);
}

void send_msg(int req, char* buf, pid_t pid) {
    int msqid;

    if((msqid = get_client_msqid_by_pid(pid)) < 0) {
        printf("Cannot open queue for PID %d\n", pid);
        return;
    }

    message_buf newMsg;
    newMsg.senderPid = getpid();
    sprintf(newMsg.mtext, "%s", buf);
    newMsg.mtype = req;

    if (msgsnd(msqid, &newMsg, MSGBUF_SIZE, 0) < 0) {
        printf("Cannot send message to %d msqid\n", msqid);
        return;
    }
}

void time_service(message_buf buf) {
    time_t timeval;
    time(&timeval);
    struct tm * localtimeval = localtime(&timeval);
    strcpy(buf.mtext, asctime(localtimeval));
    int ind = (int) strlen(buf.mtext)-1;
    if (buf.mtext[ind] == '\n') buf.mtext[ind] = '\0';
    echo_service(buf);
}

void terminate() {
    char buf[1];
    buf[0] = '\0';
    for (int i=0; i<clientCounter; i++) {
        send_msg(TERMINATE_REQ, buf, clients[i].pid);
    }
    printf("Server termination\n");
    remove_queue();
}

void client_logout(message_buf buf) {
    int ind = 0, change = 0;
    char mbuf[1];
    mbuf[0] = '\0';
    send_msg(TERMINATE_REQ, mbuf, buf.senderPid);

    for (int i=0; i<clientCounter-1; i++) {
        if(clients[i].pid != buf.senderPid) ind++;
        else change = 1;
        if(change == 1) clients[ind] = clients[i+1];
    }

    clientCounter--;
    show_clients();
}

void register_client(char * key_str, pid_t pid) {
    if (clientCounter >= CLIENTS_MAX_NUM) {
        perror("Cannot add new client. Limit has expired.\n");
        return;
    }
    int msqid;
    size_t buf_length;
    key_t key = (key_t) atol(key_str);
    client clientStr;

    if ((msqid = msgget(key, 0666)) < 0) {
        perror("msgget: Opening server queue failed");
        exit(EXIT_FAILURE);
    }

    clientStr.pid = pid;
    clientStr.key = key;
    clients[clientCounter] = clientStr;

    message_buf newMsg;
    newMsg.senderPid = getpid();
    sprintf(newMsg.mtext, "%d", idCounter++);
    newMsg.mtype = REGISTER_OK;
    buf_length = strlen(newMsg.mtext) + 1 ;

    if (msgsnd(msqid, &newMsg, buf_length, 0) < 0) {
        printf("Cannot send message to %d msqid\n", msqid);
        return;
    }
    clientCounter++;
    printf("Client with PID %d no %d has been registered\n", clientStr.pid, clientCounter);
    show_clients();
}

void remove_queue() {
    if(msgctl(serverMsqid, IPC_RMID, NULL) < 0){
        perror("msgctl error: cannot remove queue\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void show_clients() {
    printf(ANSI_COLOR_MAGENTA "Clients number: %d" ANSI_COLOR_RESET "\n", clientCounter);
    for (int i=0; i<clientCounter; i++) {
        printf(ANSI_COLOR_MAGENTA "\tno.: %d, key: %d, pid: %d" ANSI_COLOR_RESET "\n", i, clients[i].key, clients[i].pid);
    }
}