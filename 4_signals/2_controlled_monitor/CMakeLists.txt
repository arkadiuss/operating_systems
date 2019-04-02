cmake_minimum_required(VERSION 2.8)
project(ControlledMonitor C)


add_definitions(-Wall -std=c99 -D_POSIX_C_SOURCE)
set(CMAKE_C_STANDARD 99)

add_executable(cmonitor main.c monitor.c child_manager.c child_manager.h)
