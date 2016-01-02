cmake_minimum_required(VERSION 3.3)
project(Lab4_V2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp pager.cpp pager.h)
add_executable(Lab4_V2 ${SOURCE_FILES})