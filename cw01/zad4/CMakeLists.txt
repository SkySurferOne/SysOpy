cmake_minimum_required(VERSION 3.6)
project(cw4)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

link_directories(../zad1/build)

include_directories(../zad1 include src)

set(SOURCE_FILES main.c)

add_executable(main ${SOURCE_FILES})

target_link_libraries(main addressbookStatic)