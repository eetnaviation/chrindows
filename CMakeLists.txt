cmake_minimum_required(VERSION 3.30)
project(chrindows)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(chrindows main.cpp)

target_link_libraries(chrindows winmm)