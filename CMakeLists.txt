cmake_minimum_required(VERSION 3.10)
project(DaemonExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_executable(daemon_example lab1/main.cpp lab1/daemon.cpp lab1/config.cpp)

