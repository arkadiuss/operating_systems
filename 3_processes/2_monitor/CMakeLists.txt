cmake_minimum_required(VERSION 3.13)
project(Monitor C)

add_definitions(-Wall)
set(CMAKE_C_STANDARD 99)

add_executable(monitor main.c monitor.c)