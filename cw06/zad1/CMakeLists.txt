cmake_minimum_required(VERSION 3.5)
project(zad1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SERVER_SOURCE_FILES common.h server.c)
set(CLIENT_SOURCE_FILES common.h client.c)
add_executable(server ${SERVER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})