cmake_minimum_required(VERSION 3.5)
project(zad2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SOURCE_FILES main.c)
set(SOURCE_FILES_CPU_TEST cpuTimeTest.c)
set(SOURCE_FILES_MEM_TEST memTest.c)

add_executable(zad2 ${SOURCE_FILES})
add_executable(cputimetest ${SOURCE_FILES_CPU_TEST})
add_executable(memtest ${SOURCE_FILES_MEM_TEST})