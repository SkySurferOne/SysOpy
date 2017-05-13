cmake_minimum_required(VERSION 3.5)
project(zad2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror -pthread")

set(BARBER_SOURCE_FILES common.h common.c barber.c)
set(CLIENT_SOURCE_FILES common.h common.c client.c)
add_executable(barber ${BARBER_SOURCE_FILES})
add_executable(client ${CLIENT_SOURCE_FILES})
target_link_libraries(barber -lrt)
target_link_libraries(client -lrt)