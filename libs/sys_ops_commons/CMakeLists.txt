cmake_minimum_required(VERSION 3.13)
project(sys_ops_commons C)

set(CMAKE_C_STANDARD 99)

add_library(sys_ops_commons sys-ops-commons.c sys-ops-commons.h)