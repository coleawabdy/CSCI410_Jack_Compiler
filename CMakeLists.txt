cmake_minimum_required(VERSION 3.21)
project(CSCI410_Jack_Compiler)

include(FetchContent)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

# CXX Parameters
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Target
set(COMPILER_TARGET "compiler")
set(COMPILER_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/include")

# fmt library
FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG master)
FetchContent_MakeAvailable(fmt)

set(COMPILER_SOURCES
        src/main.cpp
        src/token.cpp
        src/compiler.cpp
        src/tokenizer.cpp
        src/lexer.cpp
        src/generator.cpp)

add_executable(${COMPILER_TARGET} ${COMPILER_SOURCES})

target_include_directories(${COMPILER_TARGET} PUBLIC ${COMPILER_INCLUDE})
target_link_libraries(${COMPILER_TARGET} PUBLIC fmt::fmt)
