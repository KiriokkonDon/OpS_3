cmake_minimum_required(VERSION 3.10)
project(CrossPlatformLogger)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(CrossPlatformLogger main.cpp)


if(WIN32)
    add_definitions(-DIS_WINDOWS)
else()
    add_definitions(-D_POSIX_C_SOURCE=200809L)
endif()


target_link_libraries(CrossPlatformLogger pthread)
