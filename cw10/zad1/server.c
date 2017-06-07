#include "common.h"

// structs
typedef struct ParsedArgs {
    char *exePath;
    in_port_t port;
    char path[PATH_MAX];
} ParsedArgs;

typedef struct Client {
    int fd;
    int ponged;
} Client;

// globals
int server_unix_desc, server_inet_desc, epoll_fd, clients_num = 0;
Client clients[CLIENT_MAX_NUM];
pthread_t net_thread, ping_thread;
ParsedArgs *parsed_args;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
struct epoll_event events[MAX_EPOLL_EVENTS];

// api
void parse_args(int, char **);
void init_inet_soc();
void init_unix_soc();
void init();
void make_thread();
void loop();
int soc_nonblock(int);
void * net_routine(void *);
void perform_opt(int opt);
void help();
void add_client(int);
void close_client(int);
int get_client();
void clear();
void sigint_handler(int);
void print_clients();
void * ping_routine(void *);
void set_ponged(int);

int main(int argc, char **argv) {
    parsed_args = malloc(sizeof(ParsedArgs));
    parse_args(argc, argv);
    init();
    make_thread();
    loop();

    return 0;
}

void help() {
    printf("Write: \n"
                   "\t1 - to eval a + b\n"
                   "\t2 - to eval a - b\n"
                   "\t3 - to eval a * b\n"
                   "\t4 - to eval a / b\n"
                   "\t5 - to exit\n"
                   "\tother - help\n");
}

void perform_opt(int opt) {
    fflush(stdout);

    int a, b;
    message msg;
    msg.msg_type = PERFORM_TASK;
    msg.task = opt;

    if (opt == EXIT)
        exit(EXIT_SUCCESS);

    if (opt != SUM && opt != SUB && opt != MUL && opt != DIV) {
        help();
        fflush(stdout);
        return;
    }

    int f = get_client();

    if (f == -1) {
        printf("There is no clients.\n");
        fflush(stdout);
        return;
    }
    msg.id = f;

    printf("Write a and b:\n");
    fflush(stdout);

    if (scanf("%d %d", &a, &b) == EOF) {
        perror("Error occured in scanf");
        return;
    }

    msg.a = a;
    msg.b = b;

    if (send(f, &msg, sizeof(msg), 0) == -1) {
        perror("send error");
    }

}

void * ping_routine(void *args) {
    message msg;
    msg.msg_type = PING;

    while(1) {
        pthread_mutex_lock(&client_mutex);

        for(int i = 0; i < clients_num; i++) {
            clients[i].ponged = 0;
            msg.id = clients[i].fd;
            if (send(clients[i].fd, &msg, sizeof(msg), 0) == -1) {
                perror("send error");
            }
        }

        pthread_mutex_unlock(&client_mutex);
        sleep(SLEEP_AFTER_PING);

        // TODO fix this
        pthread_mutex_lock(&client_mutex);
        int *to_delete = calloc(clients_num, sizeof(int));
        int j = 0;
        for(int i = 0; i< clients_num; i++) {
            if (clients[i].ponged == 0) {
                to_delete[j++] = clients[i].fd;
            }
        }
        pthread_mutex_unlock(&client_mutex);

        for (int i=0; i<j; i++) {
            close_client(to_delete[i]);
        }
        free(to_delete);
    }
}

int get_client() {
    srand((unsigned int) time(NULL));

    pthread_mutex_lock(&client_mutex);

    if (clients_num == 0) {
        pthread_mutex_unlock(&client_mutex);
        return -1;
    }

    int r = rand() % clients_num;
    int fd = clients[r].fd;

    pthread_mutex_unlock(&client_mutex);

    return fd;
}

void loop() {
    help();
    fflush(stdout);

    int opt;
    while(1) {
        printf(">");
        fflush(stdout);

        if (scanf(" %d", &opt) == EOF)
            break;
        getchar();
        fflush(stdin);


        printf("opt: %d\n", opt);
        fflush(stdout);
        perform_opt(opt);
    }

}

void init() {
    atexit(clear);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
}

void make_thread() {
    if (pthread_create(&net_thread, NULL, net_routine, NULL) == -1) {
        perror("pthread_create error - net_thread");
        exit(EXIT_FAILURE);
    }

    if(pthread_create(&ping_thread, NULL, ping_routine, NULL)){
        perror("pthread_create error - net_thread");
        exit(EXIT_FAILURE);
    }
}

int soc_nonblock(int fd) {
    int flags, res;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return EXIT_FAILURE;

    flags |= O_NONBLOCK;

    res = fcntl(fd, F_SETFL, flags);
    if (res == -1)
        return EXIT_FAILURE;

    return 0;
}

void close_client(int fd) {
    pthread_mutex_lock(&client_mutex);

    for (int i = 0, j = 0; i < clients_num; i++) {
        if (clients[i].fd != fd) {
            clients[j].fd = clients[i].fd;
            clients[j++].ponged = clients[i].ponged;
        }
        else {
            if (close(fd) == -1) {
                perror("close error - cannot close client fd");
            }
        }
    }

    clients_num--;
    print_clients();
    pthread_mutex_unlock(&client_mutex);
}

void print_clients() {
    printf("Connected clients:\n");
    for (int i=0; i<clients_num; i++) {
        printf("\t(%d) fd: %d\n", i+1, clients[i].fd);
    }
    printf("\tTotal: %d\n", clients_num);
}

void add_client(int fd) {
    pthread_mutex_lock(&client_mutex);

    if (clients_num >= CLIENT_MAX_NUM) {
        perror("Too many clients");
        pthread_mutex_unlock(&client_mutex);
        exit(EXIT_FAILURE);
    }

    clients[clients_num].fd = fd;
    clients[clients_num++].ponged = -1;
    print_clients();
    pthread_mutex_unlock(&client_mutex);
}

void * net_routine(void *arg) {
    struct epoll_event event;

    // initialize sockets
    init_inet_soc();
    init_unix_soc();

    if (soc_nonblock(server_inet_desc) == -1) {
        perror("soc_nonblock error - setting nonblock inet failed");
        exit(EXIT_FAILURE);
    }

    if (soc_nonblock(server_unix_desc) == -1) {
        perror("soc_nonblock error - setting nonblock unix failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_inet_desc, SOMAXCONN) == -1) {
        perror("failed listening on internet socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_unix_desc, SOMAXCONN) == -1) {
        perror("listen error");
        exit(EXIT_FAILURE);
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("error epoll_create1");
        exit(EXIT_FAILURE);
    }

    event.data.fd = server_inet_desc;
    event.events = EPOLLIN | EPOLLRDHUP;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_inet_desc, &event) == -1) {
        perror("epoll_ctl error - adding inet to epool failed");
        exit(EXIT_FAILURE);
    }

    event.data.fd = server_unix_desc;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_unix_desc, &event) == -1) {
        perror("epoll_ctl error - adding unix to epool failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int n = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, -1);
        if (n == -1) {
            perror("epoll_wait");
            return NULL;
        }

        for (int i = 0; i < n; i++) {
            event = events[i];
            if ((event.events & EPOLLERR) != 0 ||
                (event.events & EPOLLHUP) != 0) {

                perror("epoll fd error");
                if (event.data.fd != server_inet_desc && event.data.fd != server_unix_desc) {
                    close_client(event.data.fd);
                }

            }
            else if ((event.events & EPOLLRDHUP) != 0) {
                if (event.data.fd != server_inet_desc &&
                    event.data.fd != server_unix_desc) {
                    close_client(event.data.fd);
                }

            }
            else if (event.data.fd == server_inet_desc ||
                     event.data.fd == server_unix_desc) {

                while (1) {
                    struct sockaddr in_addr;
                    socklen_t len = sizeof(in_addr);
                    struct epoll_event event2;

                    int fd = accept(event.data.fd, &in_addr, &len);
                    if (fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } else {
                            perror("accept error");
                            break;
                        }
                    }

                    if (soc_nonblock(fd) == -1) {
                        perror("error soc_nonblock");
                        exit(EXIT_FAILURE);
                    }

                    event2.events = EPOLLIN | EPOLLET;
                    event2.data.fd = fd;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event2) == -1) {
                        perror("failed to add incoming socket to epoll");
                        exit(EXIT_FAILURE);
                    }

                    add_client(fd);
                }
            }
            else {
                while (1) {
                    message msg;
                    ssize_t count = recv(event.data.fd, &msg, sizeof(msg), MSG_WAITALL);

                    if (count == -1) {
                        if (errno != EAGAIN) {
                            perror("recv error");
                            close_client(event.data.fd);
                        }
                        break;
                    }
                    else if (count == 0) {
                        close_client(event.data.fd);
                        break;
                    } else if (count < sizeof(msg)) {
                        perror("Received partial message");
                        continue;
                    }
                    else {
                        switch (msg.msg_type) {
                            case RESULT:
                                printf("Result <%d> = %d\n", msg.id, msg.result);
                                fflush(stdout);
                                break;
                            case KILL:
                                close_client(event.data.fd);
                                break;
                            case CONNECT:
                                printf("%s connected\n", msg.name);
                                break;
                            case PONG:
                                // printf("%d ponged\n", event.data.fd);
                                set_ponged(event.data.fd);
                                break;
                            default:
                                printf("unknown message type\n");
                                break;
                        }
                    }
                }
            }
        }
    }

    perror("thread finished work\n");
    return NULL;
}

void set_ponged(int fd) {
    for (int i=0; i<clients_num; i++) {
        if (clients[i].fd == fd) {
            clients[i].ponged = 1;
            break;
        }
    }
}

void parse_args(int argc, char **argv) {
    parsed_args->exePath = argv[0];
    if (argc < 3) {
        printf("Too less arguments. Write <port> and <path>\n");
        exit(EXIT_FAILURE);
    }

    parsed_args->port = (in_port_t) atoi(argv[1]);
    strncpy(parsed_args->path, argv[2], PATH_MAX);
    parsed_args->path[PATH_MAX - 1] = '\0';
}

void init_inet_soc() {
    struct sockaddr_in sockaddr_inet_serv;
    memset(&sockaddr_inet_serv, 0, sizeof(sockaddr_inet_serv));

    if ((server_inet_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket eror");
        exit(EXIT_FAILURE);
    }

    sockaddr_inet_serv.sin_family = AF_INET;
    sockaddr_inet_serv.sin_addr.s_addr = INADDR_ANY;
    sockaddr_inet_serv.sin_port = htons((uint16_t) parsed_args->port);

    if (bind(server_inet_desc, (const struct sockaddr *) &sockaddr_inet_serv, sizeof(sockaddr_inet_serv)) == -1) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }
}

void init_unix_soc() {
    struct sockaddr_un sockaddr_un_serv;
    memset(&sockaddr_un_serv, 0, sizeof(sockaddr_un_serv));

    if ((server_unix_desc = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    sockaddr_un_serv.sun_family = AF_UNIX;
    strncpy(sockaddr_un_serv.sun_path, parsed_args->path, PATH_MAX);
    unlink(sockaddr_un_serv.sun_path);

    if (bind(server_unix_desc, (struct sockaddr *) &sockaddr_un_serv, sizeof(sockaddr_un_serv)) == -1) {
        perror("bind error");
        exit(EXIT_FAILURE);
    }
}

void clear() {
    if ((errno = pthread_cancel(net_thread)) == -1) {
        perror("pthread_cancel error - net_thread");
    }

    if ((errno = pthread_join(net_thread, NULL)) == -1) {
        perror("pthread_join error - net_thread");
    }

    if ((errno = pthread_cancel(ping_thread)) == -1) {
        perror("pthread_cancel error - ping_thread");
    }

    if ((errno = pthread_join(ping_thread, NULL)) == -1) {
        perror("pthread_join error - ping_thread");
    }

    if (epoll_fd != -1) {
        if (close(epoll_fd) == -1) {
            perror("error close - closing epoll fd failed");
        }

        epoll_fd = -1;
    }

    if (server_unix_desc != -1) {
        if (shutdown(server_unix_desc, SHUT_RDWR) == -1) {
            perror("shutdown error - unix shutdown failed");
        }

        if (close(server_unix_desc) == -1) {
            perror("shutdown error - closing unix socket failed");
        }

        if (unlink(parsed_args->path) == -1) {
            perror("error unlink - unix");
        }

        server_unix_desc = -1;
    }

    if (server_inet_desc != -1) {
        if (shutdown(server_inet_desc, SHUT_RDWR) == -1) {
            perror("failed shutting down internet socket");
        }

        if (close(server_inet_desc) == -1) {
            perror("failed closing internet socket");
        }

        server_inet_desc = -1;
    }

    pthread_mutex_destroy(&client_mutex);

    free(parsed_args);
}

void sigint_handler(int sig) {
    exit(EXIT_SUCCESS);
}