cmake_minimum_required(VERSION 2.8.4)
project(fixed_size_function)

set(CMAKE_CXX_FLAGS " ${CMAKE_CXX_FLAGS} -std=c++0x")
set(SOURCE_FILES tests.cpp)
add_executable(fixed_size_function ${SOURCE_FILES})