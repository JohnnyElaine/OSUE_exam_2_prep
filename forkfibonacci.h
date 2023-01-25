//
// Created by elias on 24.01.23.
//

#ifndef OSUE_EXAM_2_PREP_FORKFIBONACCI_H
#define OSUE_EXAM_2_PREP_FORKFIBONACCI_H

#define READ 0
#define WRITE 1

#include <stdbool.h>

typedef struct {
    bool is_forking;
    long value;
} config_t;

typedef struct {
    int pid;
    int stdin_fd[2];
    int stdout_fd[2];
} child_t;

long fib_memoization(long n);
long fib_fork(long n);

#endif //OSUE_EXAM_2_PREP_FORKFIBONACCI_H
