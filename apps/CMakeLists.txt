cmake_minimum_required(VERSION 3.26)
project(simple_mf_xaudio_play)

set(CMAKE_CXX_STANDARD 20)

include_directories(
        ../libs/common/include
        ../libs/xaudio/include)

add_executable(simple_mf_xaudio_play main.cpp)