cmake_minimum_required(VERSION 3.10)
project(CpuFlowPoc LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Find OpenMP
find_package(OpenMP)

# Add the source files
add_executable(benchmark src/main.cpp)

# If OpenMP is found, link it and define a macro
if(OpenMP_CXX_FOUND)
    target_link_libraries(benchmark PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(benchmark PRIVATE -DWITH_OPENMP)
else()
    message(WARNING "OpenMP not found, building without OpenMP support.")
endif()


