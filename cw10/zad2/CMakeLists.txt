cmake_minimum_required(VERSION 3.5)
project(zad2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -pthread")

set(SERVER_SOURCE_FILES server.c common.h)
add_executable(server ${SERVER_SOURCE_FILES})

set(CLIENT_SOURCE_FILES client.c common.h)
add_executable(client ${CLIENT_SOURCE_FILES})