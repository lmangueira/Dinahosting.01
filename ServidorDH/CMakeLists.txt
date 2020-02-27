cmake_minimum_required(VERSION 3.15)
project(ServidorDH C)

set(CMAKE_C_STANDARD 99)

add_executable(ServidorDH main.c)
target_link_libraries(ServidorDH wsock32 ws2_32)
