cmake_minimum_required(VERSION 3.13)
project(2_monitor C)

set(CMAKE_C_STANDARD 99)

add_executable(2_monitor main.c monitor.c monitor.h)