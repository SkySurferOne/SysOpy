cmake_minimum_required(VERSION 3.5)
project(zad2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(MAIN_SOURCE_FILES main.c)
set(MASTER_SOURCE_FILES master.c)
set(SLAVE_SOURCE_FILES slave.c)
add_executable(main ${MAIN_SOURCE_FILES})
add_executable(master ${MASTER_SOURCE_FILES})
add_executable(slave ${SLAVE_SOURCE_FILES})

target_link_libraries(slave m)