cmake_minimum_required(VERSION 3.5)
project(zad2b)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

set(SOURCE_FILES main.c)
add_executable(zad2b ${SOURCE_FILES})