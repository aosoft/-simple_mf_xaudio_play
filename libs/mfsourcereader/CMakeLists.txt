cmake_minimum_required(VERSION 3.26)
project(mfsourcereader)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")

include_directories(include ../common/include)

add_library(mfsourcereader )