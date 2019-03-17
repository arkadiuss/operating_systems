cmake_minimum_required(VERSION 3.13)
project(FunctionsComparision C)

add_definitions(-Wall)
set(CMAKE_C_STANDARD 99)
set(SOURCES main.c files_operations.c files_operations.h)
add_executable(fo ${SOURCES})