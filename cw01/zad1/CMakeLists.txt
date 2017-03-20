cmake_minimum_required(VERSION 3.5)
project(cw1_refactor)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SOURCE_FILES addressbook.h bst.h bst.c linkedlist.h linkedlist.c common.h common.c)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)
add_library(addressbookShared SHARED ${SOURCE_FILES})
add_library(addressbookStatic STATIC ${SOURCE_FILES})