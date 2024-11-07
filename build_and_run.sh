#!/bin/bash

# Check for the correct number of arguments
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <cpu|cuda|metal>"
    exit 1
fi

# Set the directory based on the argument
case "$1" in
    cpu)
        dir="cpu-flow"
        ;;
    cuda)
        dir="cuda-flow"
        ;;
    metal)
        dir="metal-flow"
        ;;
    *)
        echo "Invalid option: $1. Use cpu, cuda, or metal."
        exit 1
        ;;
esac

# Navigate to the specified directory and build
cd "$dir" || exit
mkdir -p build
cmake -S . -B build
cmake --build build

# Run the executable if it exists
if [ -f "./build/benchmark" ]; then
    ./build/benchmark
else
    echo "Executable not found. Check if the build succeeded."
    exit 1
fi