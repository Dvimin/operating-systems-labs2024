cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall -Werror")

project(daemon VERSION 0.1.0 LANGUAGES C CXX)

add_executable(daemon config.cpp daemon.cpp main.cpp)