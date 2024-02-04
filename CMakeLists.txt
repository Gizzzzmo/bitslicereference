cmake_minimum_required(VERSION 3.19.0)
project("Bit Slice Reference")

set(CMAKE_CXX_STANDARD 20)





add_executable(tests
    tests.cpp
    bitsliceref.hpp
)

target_compile_options(tests
    PUBLIC
        -fdiagnostics-color=always
        $<$<CONFIG:Debug>:--debug -O0 -Wall -gdwarf-4>
        $<$<CONFIG:Release>:-O3>
)