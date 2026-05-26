cmake_minimum_required(VERSION 3.10)
project(BShell CXX)

set(CMAKE_CXX_STANDARD 20)

find_path(READLINE_INCLUDE_DIR NAMES readline/readline.h REQUIRED)
find_library(READLINE_LIBRARY NAMES readline REQUIRED)

add_executable(BShell main.cpp
        config.cpp
        config.h
        parser.cpp
        parser.h
        Shell.cpp
        Shell.h
)

target_include_directories(BShell PRIVATE ${READLINE_INCLUDE_DIR})
target_link_libraries(BShell PRIVATE ${READLINE_LIBRARY})
