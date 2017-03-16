cmake_minimum_required(VERSION 3.6)
project(cw2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

add_subdirectory(lib)

set(SOURCE_FILES main.c)

add_executable(main ${SOURCE_FILES})

target_link_libraries(main addressbook)