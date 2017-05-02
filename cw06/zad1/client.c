#include <zconf.h>
#include <signal.h>
#include "common.h"

// api
int make_queue();
void remove_queue();
void set_server_queue();
void register_on_server(int);
void wait_for_registration();
void listen_res();
void open_req_loop();
void show_help();
void send_msg(int, char*);
void response_handler(message_buf);

// globals
int clientId = UNREGISTERED;
int serverMsqid = -1;
int clientMsqid = -1;
pid_t globalppid = -1;

int main() {
    pid_t pid;

    globalppid = getpid();
    printf(ANSI_COLOR_YELLOW "[DEBUG] client PID: %d" ANSI_COLOR_RESET "\n", globalppid);

    set_server_queue();
    key_t clientKey = make_queue();
    register_on_server(clientKey);
    wait_for_registration();

    pid = fork();

    if(pid == 0)
        listen_res();
    else
        open_req_loop();

    return 0;
}

key_t make_queue() {
    key_t clientKey;
    int msgflg, msqid;

    if ((clientKey = ftok(getenv("HOME"), globalppid)) == -1) {
        perror("ftok: generating IPC key failed");
        exit(EXIT_FAILURE);
    }

    msgflg = IPC_CREAT | 0666;
    if((msqid = msgget(clientKey, msgflg)) == -1) {
        perror("msgget: making queue failed");
        exit(EXIT_FAILURE);
    }

    clientMsqid = msqid;

    return clientKey;
}

void register_on_server(key_t clientKey) {
    char buf[MSGSZ];
    sprintf(buf, "%d", clientKey);
    printf(ANSI_COLOR_YELLOW "[DEBUG] sending client key: %s" ANSI_COLOR_RESET "\n", buf);
    send_msg(REGISTER_REQ, buf);
}

void send_msg(int req, char* buf) {
    message_buf sbuf;
    size_t buf_length;

    sbuf.mtype = req;
    //sbuf.clientId = clientId;
    sbuf.senderPid = globalppid;
    sprintf(sbuf.mtext, "%s", buf);

    buf_length = strlen(sbuf.mtext) + 1;

    if (serverMsqid == -1 || msgsnd(serverMsqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        printf ("%d, %ld, %s, %ld\n", serverMsqid, sbuf.mtype, sbuf.mtext, buf_length);
        perror("msgsnd: sending message failed");
        exit(EXIT_FAILURE);
    }
    else {
        printf("Message: \"%s\" has been sent\n", sbuf.mtext);
    }
}

void set_server_queue() {
    int msqid;
    char* envHome = getenv("HOME");
    key_t serverKey;

    if ((serverKey = ftok(envHome, SERVER_KEY)) == -1) {
        perror("ftok: generating IPC key failed");
        exit(EXIT_FAILURE);
    }

    if ((msqid = msgget(serverKey, 0666)) < 0) {
        perror("msgget: Opening server queue failed");
        exit(EXIT_FAILURE);
    }

    serverMsqid = msqid;
    printf(ANSI_COLOR_YELLOW "[DEBUG] server_key %d, server_msqid: %d" ANSI_COLOR_RESET "\n", serverKey, serverMsqid);
}

void wait_for_registration() {
    message_buf  rbuf;
    struct msqid_ds buf;

    printf("Waiting for registration.\n");
    while(1) {
        msgctl(clientMsqid, IPC_STAT, &buf);

        if(buf.msg_qnum != 0) {
            if (msgrcv(clientMsqid, &rbuf, MSGSZ, 0, IPC_NOWAIT) < 0) {
                perror("msgrcv: receiving message failed");
                exit(EXIT_FAILURE);
            }

            if (rbuf.mtype == REGISTER_OK) {
                printf("Registration successful. Received id from server: %s\n", rbuf.mtext);
                clientId = atoi(rbuf.mtext);
                break;
            } else {
                perror("undefined mtype - registration failed");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void open_req_loop() {
    char cmd;
    char buf [MSGSZ];

    printf("Write command. Write 'h' if you need help.\n");
    while(1) {
        printf("> ");
        scanf(" %c", &cmd);
        getchar();

        if(cmd == 'e' || cmd == 'u') {
            fflush(stdin);
            fgets(buf, sizeof(buf), stdin);
            int ind = (int) strlen(buf)-1;
            if(buf[ind] == '\n') buf[ind] = '\0';
        } else {
            buf[0] = '\0';
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
        if(cmd == 'q') break;
        fflush(stdin);
    }
    while(1){}; // wait for kill
}

void listen_res() {
    message_buf  rbuf;
    struct msqid_ds buf;

    while(1) {
        msgctl(clientMsqid, IPC_STAT, &buf);

        if(buf.msg_qnum != 0) {
            if (msgrcv(clientMsqid, &rbuf, MSGSZ, 1, 0) < 0) {
                perror("msgrcv: receiving message failed");
                exit(EXIT_FAILURE);
            }

            response_handler(rbuf);
        }
    }
}

void response_handler(message_buf buf) {
    if(buf.mtype == PRINT) {
        printf("Response: %s\n", buf.mtext);
    } else {
        kill(getppid(), SIGKILL);
        remove_queue();
        exit(EXIT_SUCCESS);
    }
}

void remove_queue() {
    if(msgctl(clientMsqid, IPC_RMID, NULL) < 0){
        printf("msgctl error: cannot remove queue\n");
        exit(EXIT_FAILURE);
    }
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