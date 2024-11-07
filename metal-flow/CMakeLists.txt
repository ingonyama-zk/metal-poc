cmake_minimum_required(VERSION 3.15)
project(MetalFlowPoc)

# Optionally set default build type if not provided
# set(CMAKE_BUILD_TYPE "Profiling" CACHE STRING "Build type for profiling with gprof" FORCE)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(MetalFlow ${PROJECT_SOURCE_DIR}/src/main.cpp)

# Profiling flags for gprof
if(CMAKE_BUILD_TYPE STREQUAL "Profiling")
    target_compile_options(MetalFlow PRIVATE -pg)
    target_link_options(MetalFlow PRIVATE -pg)
endif()

# Link the necessary frameworks
target_link_libraries(MetalFlow
    PRIVATE
        "-framework Metal"
        "-framework Foundation"
)

# Specify the include directories
target_include_directories(MetalFlow PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Add custom command to compile Metal shader
add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/shader.air
    COMMAND xcrun -sdk macosx metal -c ${PROJECT_SOURCE_DIR}/shaders/shader.metal -o ${PROJECT_BINARY_DIR}/shader.air
    DEPENDS ${PROJECT_SOURCE_DIR}/shaders/shader.metal
    COMMENT "Compiling Metal shader"
)

add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/default.metallib
    COMMAND xcrun -sdk macosx metallib ${PROJECT_BINARY_DIR}/shader.air -o ${PROJECT_BINARY_DIR}/default.metallib
    DEPENDS ${PROJECT_BINARY_DIR}/shader.air
    COMMENT "Creating Metal library"
)

# Make sure the shader is built before the executable
add_custom_target(shaders DEPENDS ${PROJECT_BINARY_DIR}/default.metallib)
add_dependencies(MetalFlow shaders)

# Pass the metallib file path to the executable
set(METAL_LIBRARY_PATH "${PROJECT_BINARY_DIR}/default.metallib")
target_compile_definitions(MetalFlow PRIVATE METAL_LIBRARY_PATH="${METAL_LIBRARY_PATH}")