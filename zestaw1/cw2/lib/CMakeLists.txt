cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES addressbook.h bst.h bst.c linkedlist.h linkedlist.c common.h common.c)

add_library(addressbook ${SOURCE_FILES})