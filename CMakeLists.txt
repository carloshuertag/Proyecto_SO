cmake_minimum_required(VERSION 3.19)
project(Proyecto_SO C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Proyecto_SO
        control.c
        store.h)
