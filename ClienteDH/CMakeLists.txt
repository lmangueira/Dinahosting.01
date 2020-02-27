cmake_minimum_required(VERSION 3.15)
project(ClienteDH C)

set(CMAKE_C_STANDARD 99)

include_directories(include)
link_directories(lib)

add_executable(ClienteDH main.c)
target_link_libraries(ClienteDH libmysql wsock32 ws2_32)
