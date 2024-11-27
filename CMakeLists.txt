
cmake_minimum_required(VERSION 3.31)
project(LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

install(DIRECTORY include/ DESTINATION /usr/include/TenSore)

add_executable(Example main.cpp)
