cmake_minimum_required(VERSION 3.21)
project(CSCI410_Jack_Compiler)

# CXX Parameters
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Target
set(COMPILER_TARGET "compiler")
set(COMPILER_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/include")

set(COMPILER_SOURCES
        src/main.cpp
        src/compiler.cpp
        src/tokenizer.cpp)

add_executable(${COMPILER_TARGET} ${COMPILER_SOURCES})

target_include_directories(${COMPILER_TARGET} PUBLIC ${COMPILER_INCLUDE})

