cmake_minimum_required(VERSION 2.8)
project(1_command_executor C)

set(CMAKE_C_STANDARD 99)
set(commons ../../libs/sys_ops_commons)

include_directories(${commons})
add_library(sys_ops_commons ${commons}/sys-ops-commons.c)
add_definitions(-Wall -std=c99)
add_executable(1_command_executor main.c)
target_link_libraries(1_command_executor sys_ops_commons)