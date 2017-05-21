cmake_minimum_required(VERSION 3.5)
project(zad1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -pthread")

set(SOURCE_FILES utils.h utils.c main.c)
set(UTILS_TEST_FILES utils.h utils.c utils_test.c)
add_executable(zad1b ${SOURCE_FILES})
add_executable(utils ${UTILS_TEST_FILES})