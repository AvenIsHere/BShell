cmake_minimum_required(VERSION 3.10)
project(BasicShell CXX)

set(CMAKE_CXX_STANDARD 20)

find_library(READLINE_LIBRARY NAMES readline)

add_executable(BasicShell main.cpp
        config.cpp
        config.h)

target_link_libraries(BasicShell PRIVATE ${READLINE_LIBRARY})
