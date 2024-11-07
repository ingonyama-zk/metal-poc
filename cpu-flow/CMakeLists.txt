cmake_minimum_required(VERSION 3.10)
project(CpuFlowPoc LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

# Add the source files
add_executable(benchmark src/main.cpp)
