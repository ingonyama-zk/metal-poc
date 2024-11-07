cmake_minimum_required(VERSION 3.10)
project(CudaFlowPOC LANGUAGES CXX CUDA)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_STANDARD 17)

# Enable CUDA support
find_package(CUDAToolkit REQUIRED)

# Add the source files
add_executable(benchmark src/main.cu)

# Include dir
target_include_directories(benchmark PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})

# Link CUDA libraries
target_link_libraries(benchmark PRIVATE ${CUDA_LIBRARIES})