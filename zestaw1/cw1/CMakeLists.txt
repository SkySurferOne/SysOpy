cmake_minimum_required(VERSION 3.0)
project(cw01)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(SOURCE_FILES addressbooklib.c addressbooklib.h test.c)
add_library(cw01Shared SHARED ${SOURCE_FILES})
add_library(cw01Static STATIC ${SOURCE_FILES})
add_executable(main_test test.c)
target_link_libraries(main_test cw01Static)
