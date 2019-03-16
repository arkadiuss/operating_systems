cmake_minimum_required(VERSION 3.13)
project(2_find_by_date C)

set(CMAKE_C_STANDARD 99)

add_executable(2_find_by_date main.c file_finder.c file_finder.h)