cmake_minimum_required(VERSION 3.24)
project(OSUE_exam_2_prep C)

set(CMAKE_C_STANDARD 99)

add_executable(server main_server.c forkfibonacci.c forkfibonacci.h)
add_executable(forkfibonacci forkfibonacci.c forkfibonacci.h)