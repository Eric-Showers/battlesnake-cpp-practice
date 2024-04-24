cmake_minimum_required(VERSION 3.28)
project(battlesnake_starter_cpp)

set(CMAKE_CXX_STANDARD 20)

include_directories(include/)

add_executable(battlesnake_starter_cpp src/main.cpp
        src/battlesnake.cpp
        include/battlesnake.h
        include/json.h
        include/httplib.h)
