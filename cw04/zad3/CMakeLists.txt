cmake_minimum_required(VERSION 3.5)
project(zad3)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SOURCE_FILES main.c)
add_executable(zad3 ${SOURCE_FILES})