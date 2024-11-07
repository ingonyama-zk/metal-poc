#!/bin/bash

set -e

# Check for the correct number of arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <cpu|cuda|metal> <log_size>"
    exit 1
fi

# Set the directory based on the first argument
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

# Get the log size from the second argument
log_size=$2

# Navigate to the specified directory and build
cd "$dir" || exit
mkdir -p build
cmake -S . -B build
cmake --build build

# Run the executable with the log_size argument if it exists
if [ -f "./build/benchmark" ]; then
    ./build/benchmark "$log_size"
else
    echo "Executable not found. Check if the build succeeded."
    exit 1
fi