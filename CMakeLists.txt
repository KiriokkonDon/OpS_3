cmake_minimum_required(VERSION 3.10)
project(CrossPlatformLogger)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(CrossPlatformLogger main.cpp)


if(WIN32)
    target_compile_definitions(CrossPlatformLogger PRIVATE IS_WINDOWS=1)
else()
    target_compile_definitions(CrossPlatformLogger PRIVATE IS_WINDOWS=0)
endif()


target_link_libraries(CrossPlatformLogger pthread)
