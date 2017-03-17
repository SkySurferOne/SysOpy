cmake_minimum_required(VERSION 3.6)
project(cw3a)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Werror")

set(CMAKE_C_STANDARD 11)

link_directories(../cw1/build)
include_directories(../cw1 include src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# compile with static lib
set(SOURCE_FILES ../cw2/main.c)
add_executable(cw3a_static ${SOURCE_FILES})
target_link_libraries(cw3a_static addressbookStatic)

# compile with shared lib
add_executable(cw3a_shared ${SOURCE_FILES})
target_link_libraries(cw3a_shared addressbookShared)

# compile with dynamic lib
set(DYN_SOURCE_FILES main_dynamic.c)
add_executable(cw3a_dynamic ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic dl)

# compile with dynamic lib with -O
add_executable(cw3a_dynamic_o  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_o dl)
set_target_properties(cw3a_dynamic_o PROPERTIES COMPILE_FLAGS -O)

# compile with dynamic lib with -O1
add_executable(cw3a_dynamic_o1  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_o1 dl)
set_target_properties(cw3a_dynamic_o1 PROPERTIES COMPILE_FLAGS -O1)

# compile with dynamic lib with -O2
add_executable(cw3a_dynamic_o2  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_o2 dl)
set_target_properties(cw3a_dynamic_o2 PROPERTIES COMPILE_FLAGS -O2)

# compile with dynamic lib with -O3
add_executable(cw3a_dynamic_o3  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_o3 dl)
set_target_properties(cw3a_dynamic_o3 PROPERTIES COMPILE_FLAGS -O3)

# compile with dynamic lib with -O0
add_executable(cw3a_dynamic_o0  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_o0 dl)
set_target_properties(cw3a_dynamic_o0 PROPERTIES COMPILE_FLAGS -O0)

# compile with dynamic lib with -Os
add_executable(cw3a_dynamic_os  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_os dl)
set_target_properties(cw3a_dynamic_os PROPERTIES COMPILE_FLAGS -Os)

# compile with dynamic lib with -Ofast
add_executable(cw3a_dynamic_ofast  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_ofast dl)
set_target_properties(cw3a_dynamic_ofast PROPERTIES COMPILE_FLAGS -Ofast)

# compile with dynamic lib with -Og
add_executable(cw3a_dynamic_og  ${DYN_SOURCE_FILES})
target_link_libraries(cw3a_dynamic_og dl)
set_target_properties(cw3a_dynamic_og PROPERTIES COMPILE_FLAGS -Og)

add_custom_target(testsOutput ALL
                  DEPENDS result.txt result_o.txt result_o1.txt result_o2.txt result_o3.txt result_o0.txt result_os.txt result_ofast.txt result_og.txt)

add_custom_command(OUTPUT result.txt result_o.txt result_o1.txt result_o2.txt result_o3.txt result_o0.txt result_os.txt result_ofast.txt result_og.txt
        COMMAND cw3a_dynamic > ${CMAKE_CURRENT_SOURCE_DIR}/result.txt
        COMMAND cw3a_dynamic_o > ${CMAKE_CURRENT_SOURCE_DIR}/result_o.txt
        COMMAND cw3a_dynamic_o1 > ${CMAKE_CURRENT_SOURCE_DIR}/result_o1.txt
        COMMAND cw3a_dynamic_o2 > ${CMAKE_CURRENT_SOURCE_DIR}/result_o2.txt
        COMMAND cw3a_dynamic_o3 > ${CMAKE_CURRENT_SOURCE_DIR}/result_o3.txt
        COMMAND cw3a_dynamic_o0 > ${CMAKE_CURRENT_SOURCE_DIR}/result_o0.txt
        COMMAND cw3a_dynamic_os > ${CMAKE_CURRENT_SOURCE_DIR}/result_os.txt
        COMMAND cw3a_dynamic_ofast > ${CMAKE_CURRENT_SOURCE_DIR}/result_ofast.txt
        COMMAND cw3a_dynamic_og > ${CMAKE_CURRENT_SOURCE_DIR}/result_og.txt
        VERBATIM
        )
