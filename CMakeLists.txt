cmake_minimum_required(VERSION 3.8)
project(final_project)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(SOURCE_FILES
    #jacobi.cpp
    jacobi_pt.cpp)

include_directories(../../fastflow-2.1.0/)

add_executable(jacobi_pt ${SOURCE_FILES})