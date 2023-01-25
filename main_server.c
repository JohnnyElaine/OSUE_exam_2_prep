#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#include "forkfibonacci.h"

#define QUEUE_SIZE 8
#define READ 0
#define WRITE 1

static bool is_running = true;

int create_socket(char *port){
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int res = getaddrinfo(NULL, port, &hints, &ai);
    if (res != 0) {
        fprintf(stderr, "%s\n", gai_strerror(res));
        exit(EXIT_FAILURE);
    }

    int socketfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (socketfd < 0) {
        fprintf(stderr, "unable to create socket\n");
        freeaddrinfo(ai);
        exit(EXIT_FAILURE);
    }

    int optval = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(socketfd, ai->ai_addr, ai->ai_addrlen) < 0) {
        fprintf(stderr, "unable to bind socket\n");
        if (errno == EADDRINUSE) fprintf(stderr, "IP-Addr already in use\n");
        freeaddrinfo(ai);
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(ai);

    if (listen(socketfd, QUEUE_SIZE) < 0) {
        fprintf(stderr, "unable to listen from socket\n");
        freeaddrinfo(ai);
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    return socketfd;
}

long convert_input_to_long(char *input){
    char *end = strpbrk(input,"\r\n");
    char copy[strlen(input)];
    strcpy(copy, input);
    if (end != NULL) copy[end - input] = '\0';

    return strtol(copy, NULL, 10);
}

void handle_request(int socket){
    int conn_fd = accept(socket, NULL, NULL);
    if (conn_fd < 0) fprintf(stderr, "Unable to accept connection");

    FILE *client_connection = fdopen(conn_fd, "r+");
    if (client_connection == NULL) fprintf(stderr, "Unable to create client filestream");

    char *line = NULL;
    size_t len = 0;
    ssize_t nread = getline(&line, &len, client_connection);
    if (nread == EOF) fprintf(stderr, "Nothing was read");

    long value = convert_input_to_long(line);
    // TODO: do something with value
    free(line);


    if (client_connection != NULL) fclose(client_connection);
}

void run_server(){
    int socket = create_socket("9999");
    while (is_running) {
        handle_request(socket);
    }
}

void handle_signal() {
    is_running = false;
}

void register_signals(void){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main() {
    register_signals();

    int fd[2];
    if (pipe(fd) < 0) fprintf(stderr, "error when calling pipe()\n");

    int pid = fork();

    if (pid == -1) {
        fprintf(stderr, "fork failed\n");
    } else if (pid == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        close(fd[0]);

        execlp("ping", "ping", "-c", "1", "google.com", NULL);

        return EXIT_SUCCESS;
    } else {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);

        int status;
        waitpid(pid, &status, 0);

        char *line = NULL;
        size_t len = 0;
        while (getline(&line, &len, stdin) != EOF) {
            printf("Message from the child process: %s", line);
        }

        if (line != NULL) free(line);
    }



    return 0;
}
