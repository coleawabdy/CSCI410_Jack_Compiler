cmake_minimum_required(VERSION 3.21)
project(CSCI410_Jack_Compiler)

# Modules
include(CMakeDependentOption)
include(FetchContent)

# Options
cmake_dependent_option(N2T_COMPLIANT_PROJECT_10 "Compliance with specifications of nand2tetris project 10" OFF "NOT N2T_COMPLIANT_PROJECT_11" OFF)
cmake_dependent_option(N2T_COMPLIANT_PROJECT_11 "Compliance with specifications of nand2tetris project 11" OFF "NOT N2T_COMPLIANT_PROJECT_10" OFF)

# TinyXML2
if(${N2T_COMPLIANT_PROJECT_10} EQUALS ON)
FetchContent_Declare(tinyxml2
        GIT_REPOSITORY  https://github.com/leethomason/TinyXML2.git
        GIT_TAG         9.0.0)
FetchContent_MakeAvailable(tinyxml2)
endif()

# CXX Parameters
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Target
set(COMPILER_TARGET "compiler")
set(COMPILER_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/include")

set(COMPILER_SOURCES
        src/main.cpp
        src/token.cpp
        src/compiler.cpp
        src/tokenizer.cpp
        src/lexer.cpp)

add_executable(${COMPILER_TARGET} ${COMPILER_SOURCES})

target_link_libraries(${COMPILER_TARGET} PUBLIC tinyxml2::tinyxml2)
target_include_directories(${COMPILER_TARGET} PUBLIC ${COMPILER_INCLUDE})

# Set compliance level
target_compile_definitions(${COMPILER_TARGET} PRIVATE
        $<$<BOOL:${N2T_COMPLIANT_PROJECT_10}>:N2T_COMPLIANT=10>
        $<$<BOOL:${N2T_COMPLIANT_PROJECT_11}>:N2T_COMPLIANT=11>)

