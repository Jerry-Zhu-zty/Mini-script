cmake_minimum_required(VERSION 4.0)
project(script_engine)

include_directories("include")

add_executable(main src/main.cpp src/script.cpp)
