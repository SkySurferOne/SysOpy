cmake_minimum_required(VERSION 3.5)
project(zad2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(UTILS_SRC_FILES utils.c utils.h)
set(SOURCE_FILES_1 main.c ${UTILS_SRC_FILES})
set(SOURCE_FILES_2 main_nftw.c ${UTILS_SRC_FILES})

add_executable(zad2 ${SOURCE_FILES_1})
add_executable(zad2_nftw ${SOURCE_FILES_2})