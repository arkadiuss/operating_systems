cmake_minimum_required(VERSION 3.13)
project(1_file_managing_library C)

set(CMAKE_C_STANDARD 99)

add_executable(1_file_managing_library main.c file_managing_library.h file_managing_library.c library_loader.h library_loader.c)