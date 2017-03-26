cmake_minimum_required(VERSION 3.5)
project(zad1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SOURCE_FILES_MAIN main.c)
set(SOURCE_FILES_TEST_ENV_VAR testEnvVar.c)

add_executable(zad1 ${SOURCE_FILES_MAIN})
add_executable(testenvvar ${SOURCE_FILES_TEST_ENV_VAR})