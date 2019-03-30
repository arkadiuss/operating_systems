cmake_minimum_required(VERSION 3.13)
project(ControlledMonitor C)

add_definitions(-Wall)
set(CMAKE_C_STANDARD 99)

add_executable(cmonitor main.c monitor.c child_manager.c child_manager.h)