cmake_minimum_required(VERSION 2.8)
set(name cluster)
project(${name} C)

set(CMAKE_C_STANDARD 99)
set(commons ../../libs/sys_ops_commons)

include_directories(${commons})
add_library(sys_ops_commons ${commons}/sys-ops-commons.c)
add_definitions(-Wall -std=c99)
add_executable(server server.c)
add_executable(client client.c)
target_link_libraries(server sys_ops_commons)
target_link_libraries(client sys_ops_commons)
