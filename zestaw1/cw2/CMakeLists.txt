cmake_minimum_required(VERSION 3.5)
project(cw2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

link_directories(../cw1/cmake-build-debug)

set(SOURCE_FILES main.c)

add_executable(main ${SOURCE_FILES})

include_directories(../cw1 include src)
target_link_libraries(main addressbookStatic)