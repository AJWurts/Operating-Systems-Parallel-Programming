cmake_minimum_required(VERSION 3.6)
project(OS_Proj3)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c bathroom.h bathroom.c bathroom2.c bathroom2.h)
add_executable(OS_Proj3 ${SOURCE_FILES})