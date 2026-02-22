cmake_minimum_required(VERSION 3.10)
project(BasicShell CXX)

set(CMAKE_C_STANDARD 17)

find_library(READLINE_LIBRARY NAMES readline)

add_executable(BasicShell main.cpp
        list.cpp
        list.h)

target_link_libraries(BasicShell PRIVATE ${READLINE_LIBRARY})
