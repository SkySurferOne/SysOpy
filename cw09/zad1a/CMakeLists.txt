cmake_minimum_required(VERSION 3.5)
project(zad2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -pthread")

set(SOURCE_FILES utils.c utils.h main.c)
add_executable(zad1a ${SOURCE_FILES})