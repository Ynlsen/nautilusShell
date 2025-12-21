cmake_minimum_required(VERSION 4.1)
project(nautilush C)

set(CMAKE_C_STANDARD 11)

add_executable(nautilush main.c)

target_link_libraries(nautilush readline)
