cmake_minimum_required(VERSION 3.13)
project(3_sender_catcher C)

set(CMAKE_C_STANDARD 99)

add_executable(catcher catcher.c common.c common.h)
add_executable(sender sender.c common.c common.h)