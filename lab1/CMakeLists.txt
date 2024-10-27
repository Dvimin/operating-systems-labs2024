cmake_minimum_required(VERSION 3.10)
project(DaemonExample)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(daemon_example main.cpp daemon.cpp config.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
