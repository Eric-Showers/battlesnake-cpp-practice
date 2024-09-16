cmake_minimum_required(VERSION 3.24)
project(battlesnake_starter_cpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug) # to do a release build use cmake -DCMAKE_BUILD_TYPE=Release ..
endif()

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-warning-option")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -Wall -Wextra -pedantic") # for memory debugging:  -fsanitize=address | if multithread: -fsanitize=thread
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG") # NDEBUG flag removes the assert macros

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # to generate a compile_commands.json used by cppcheck and sonarlint

include_directories(include/)

add_executable(battlesnake_starter_cpp src/main.cpp
        src/battlesnake.cpp
        include/battlesnake.h
        include/json.h
        include/httplib.h)
