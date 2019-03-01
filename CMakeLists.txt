cmake_minimum_required(VERSION 3.0)
project(git_hookd C)

set(CMAKE_C_STANDARD 99)

add_executable(git_hookd main.c)
target_link_libraries(git_hookd event pthread)