cmake_minimum_required(VERSION 3.5)
project(zad1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(SOURCE_FILES main.c)
add_executable(zad1 ${SOURCE_FILES})

add_custom_target(generateTestFiles)

set(4B_NUM_REC 3000)
set(512B_NUM_REC 3000)
set(4096B_NUM_REC 3000)
set(8192B_NUM_REC 3000)

add_custom_command(TARGET generateTestFiles
        POST_BUILD
        COMMAND zad1 generate -f lib_data_4B.txt ${4B_NUM_REC} 4
        COMMAND zad1 generate -f lib_data_512B.txt ${512B_NUM_REC} 512
        COMMAND zad1 generate -f lib_data_4096B.txt ${4096B_NUM_REC} 4096
        COMMAND zad1 generate -f lib_data_8192B.txt ${8192B_NUM_REC} 8192

        COMMAND cp lib_data_4B.txt sys_data_4B.txt
        COMMAND cp lib_data_512B.txt sys_data_512B.txt
        COMMAND cp lib_data_4096B.txt sys_data_4096B.txt
        COMMAND cp lib_data_8192B.txt sys_data_8192B.txt

        # test lib
        COMMAND zad1 lib sort lib_data_4B.txt ${4B_NUM_REC} 4 > wynik.txt
        COMMAND zad1 lib shuffle lib_data_4B.txt ${4B_NUM_REC} 4 >> wynik.txt

        COMMAND zad1 lib sort lib_data_512B.txt ${512B_NUM_REC} 512 >> wynik.txt
        COMMAND zad1 lib shuffle lib_data_512B.txt ${512B_NUM_REC} 512 >> wynik.txt

        COMMAND zad1 lib sort lib_data_4096B.txt ${4096B_NUM_REC} 4096 >> wynik.txt
        COMMAND zad1 lib shuffle lib_data_4096B.txt ${4096B_NUM_REC} 4096 >> wynik.txt

        COMMAND zad1 lib sort lib_data_8192B.txt ${8192B_NUM_REC} 8192 >> wynik.txt
        COMMAND zad1 lib shuffle lib_data_8192B.txt ${8192B_NUM_REC} 8192 >> wynik.txt

        #text sys
        COMMAND zad1 sys sort sys_data_4B.txt ${4B_NUM_REC} 4 >> wynik.txt
        COMMAND zad1 sys shuffle sys_data_4B.txt ${4B_NUM_REC} 4 >> wynik.txt

        COMMAND zad1 sys sort sys_data_512B.txt ${512B_NUM_REC} 512 >> wynik.txt
        COMMAND zad1 sys shuffle sys_data_512B.txt ${512B_NUM_REC} 512 >> wynik.txt

        COMMAND zad1 sys sort sys_data_4096B.txt ${4096B_NUM_REC} 4096 >> wynik.txt
        COMMAND zad1 sys shuffle sys_data_4096B.txt ${4096B_NUM_REC} 4096 >> wynik.txt

        COMMAND zad1 sys sort sys_data_8192B.txt ${8192B_NUM_REC} 8192 >> wynik.txt
        COMMAND zad1 sys shuffle sys_data_8192B.txt ${8192B_NUM_REC} 8192 >> wynik.txt

        COMMAND rm -f lib_data_4B.txt sys_data_4B.txt lib_data_512B.txt sys_data_512B.txt lib_data_4096B.txt sys_data_4096B.txt lib_data_8192B.txt sys_data_8192B.txt
        )