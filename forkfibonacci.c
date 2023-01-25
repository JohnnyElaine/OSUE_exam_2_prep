//
// Created by elias on 24.01.23.
//
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "forkfibonacci.h"

char program_name[256];

long fib_no_fork(long n, long *cache){
    if (n == 0) return 0;
    if (n == 1) return 1;


    if (cache[n] == 0) cache[n] = fib_no_fork(n - 1, cache) + fib_no_fork(n - 2, cache);

    return cache[n];
}

long fib_memoization(long n){
    long *cache = (long*) calloc(n + 2, sizeof(n));
    cache[0] = 0;
    cache[1] = 1;

    long f = fib_no_fork(n, cache);
    free(cache);
    return f;
}



void init_pipes(child_t *child){

    if (pipe(child->stdin_fd) != 0) {
        fprintf(stderr, "Unable to init pipe");
        exit(EXIT_FAILURE);
    }

    if (pipe(child->stdout_fd) != 0) {
        fprintf(stderr, "Unable to init pipe");
        exit(EXIT_FAILURE);
    }

}

void start_child(child_t *child, long n) {
    int pid = fork();
    switch (pid) {
        case -1:
            fprintf(stderr, "Error when forking\n");
            exit(EXIT_FAILURE);
            break;
        case 0:
            if (dup2(child->stdout_fd[WRITE], STDOUT_FILENO) == -1) {
                fprintf(stdout, "Error when calling dup2\n");
            }


            if (dup2(child->stdin_fd[READ], STDIN_FILENO) == -1) {
                fprintf(stdout, "Error when calling dup2\n");
            }

            close(child->stdout_fd[WRITE]);
            close(child->stdin_fd[READ]);

            // REMOVE THIS IN CASE OF ERR
            close(child->stdout_fd[READ]);
            close(child->stdin_fd[WRITE]);
            // REMOVE THIS IN CASE OF ERR

            execlp(program_name, program_name, n, NULL);

            break;
        default:
            close(child->stdin_fd[READ]);
            close(child->stdout_fd[WRITE]);
            break;
    }

}

long read_from_child(child_t *child){
    return 0;
}

long fib_fork(long n){
    if (n == 0) return 0;
    if (n == 1) return 1;

    child_t left;
    child_t right;

    init_pipes(&left);
    init_pipes(&right);

    start_child(&left, n - 1);
    start_child(&right, n - 2);

    int status;
    waitpid(left.pid, &status, 0);
    waitpid(right.pid, &status, 0);

    int left_val = read_from_child(&left);
    int right_val = read_from_child(&right);

    return 0;
}

config_t handle_arguments(int argc, char *argv[]){
    config_t config;
    config.is_forking = true;
    config.value = -1;
    int opt;

    while ((opt = getopt(argc, argv, "n")) != -1) {
        switch (opt) {
            case 'n':
                config.is_forking = false;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-n] n_fib\n",
                        program_name);
                fprintf(stderr, "-n, No forking\n");
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }

    config.value = strtol(argv[optind], NULL, 10);

    return config;
}


int main(int argc, char *argv[]) {
    strcpy(program_name, argv[0]);

    config_t config = handle_arguments(argc, argv);

    if (!config.is_forking) {
        printf("%ld\n", fib_memoization(config.value));
        exit(EXIT_SUCCESS);
    }

    long fib_value = fib_fork(config.value);

    printf("%ld\n", fib_value);


    return EXIT_SUCCESS;
}
