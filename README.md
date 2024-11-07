# Metal POC

## Building and Running

To build and run the flow, use the provided `build_and_run.sh` script. Pass in `cpu`, `cuda`, or `metal` as an argument to specify the flow you want to run:

```bash
./build_and_run.sh <cpu|cuda|metal>
```

This script will automatically navigate to the specified flow directory, create a build folder, configure and compile the project, and then execute the resulting binary.

## POC Goals

Analyze Memory Transfer Bottlenecks in Mixed Workloads and demonstrate zero cost memory transfer between CPU and GPU in Metal (for apple silicon).

### Objectives
- **Highlight CUDA Limitations**: Demonstrate the bottleneck caused by memory transfers in CUDA when CPU computation is required as a fallback.
- **Show Metal’s unified Memory Advantage**: Emphasize the efficiency of Metal’s unified CPU-GPU memory architecture, which reduces or removes the need for costly memory transfers, providing an incentive for a Metal backend for applications like ICICLE.

## POC Flows

### CUDA Flow
1. **Allocate Memory on CPU**: Initialize a sorted array of random numbers on the CPU.
2. **Copy Memory to GPU**: Transfer the sorted array to the GPU (*measure time*).
3. **Power Computation on GPU**: Raise each element to a power (e.g., square each element) while maintaining order (*measure time*).
4. **Copy Memory Back to CPU**: Transfer the modified array back to the CPU (*measure time*).
5. **Binary Search on CPU**: Perform a binary search on the powered array to check for the presence of a specific value (*measure time*).
6. **Repeat**: Execute this workflow multiple times to accumulate meaningful timing data.

### Metal Flow
1. **Allocate Memory and Initialize Data**: Initialize a sorted array of random numbers on unified memory accessible to both CPU and GPU.
2. **Power Computation on GPU**: Perform the same power operation directly on the unified memory (*measure time*).
3. **Binary Search on CPU**: Directly perform a binary search on the modified array in unified memory, eliminating the need for data copying (*measure time*).
4. **Repeat**: Iterate this flow to observe performance consistency.

In addition, we will initialize data in a CPU only buffer and measure initial copy to CPU-GPU shared buffer (*measure time*).

## Define Computations

### GPU Computation
   - **Task**: Elementwise power operation on each element in a large, sorted array (e.g., square each element).
   - **Rationale**: This is a straightforward, high-performance task for the GPU.

### CPU Computation
   - **Task**: Binary search for a specific value within the powered array.
   - **Rationale**: Binary search is inherently sequential and ideal for the CPU, illustrating the efficiency of CPU fallback for branching logic.

## Metal unified Memory Research

Before implementing the flows, it’s essential to understand Metal’s memory management to gauge potential cost and performance trade-offs:
- **Zero-Cost unified Memory**: Determine if Metal’s unified memory model truly provides zero-cost access across CPU and GPU.
- **Memory Access Patterns and Costs**: Assess if Metal offers various memory tiers (e.g., faster but more costly memory options) and if data locality impacts performance.
- **Mechanics of unified Memory**: Review Metal’s documentation to understand how unified memory is allocated, accessed, and synchronized.

[Check the summary here](./metal-programming-notes.md)

## Results

### Summary of Results for `log_size = 20`: Metal vs. CUDA

| **Configuration**                                                       | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
| ----------------------------------------------------------------------- | ---------------------------- | -------------------- | ---------------------------- | -------------------------- | ------------------------------------------- | ----------------------------------- | -------------------------------------- |
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)**               | N/A                          | 0.411 ms             | 9.58e-5 ms                   | 0.0018 ms                  | 9.58e-5 ms (0.02%)                          | 0.413 ms (99.98%)                   | 0.413 ms                               |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 0.0371 ms                    | 0.405 ms             | 8.36e-5 ms                   | 0.0015 ms                  | 0.0372 ms (8.39%)                           | 0.406 ms (91.61%)                   | 0.443 ms (accounts for initial copy)   |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)**  | 0.296 ms                     | 0.070 ms             | 0.353 ms                     | 0.0006 ms                  | 0.649 ms (90.22%)                           | 0.070 ms (9.78%)                    | 0.720 ms                               |

### Summary of Results for `log_size = 25`: Metal vs. CUDA

| **Configuration**                                                       | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
| ----------------------------------------------------------------------- | ---------------------------- | -------------------- | ---------------------------- | -------------------------- | ------------------------------------------- | ----------------------------------- | -------------------------------------- |
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)**               | N/A                          | 3.514 ms             | 0.0004 ms                    | 0.0058 ms                  | 0.0004 ms (0.01%)                           | 3.5194 ms (99.99%)                  | 3.5198 ms                              |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 3.070 ms                     | 3.022 ms             | 0.0002 ms                    | 0.0090 ms                  | 3.0700 ms (50.32%)                          | 3.0305 ms (49.68%)                  | 6.1004 ms (accounts for initial copy)  |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)**  | 8.801 ms                     | 0.416 ms             | 9.708 ms                     | 0.0005 ms                  | 18.5091 ms (97.80%)                         | 0.4161 ms (2.20%)                   | 18.9253 ms                             |


### Summary of Results for `log_size = 30`: Metal vs. CUDA

| **Configuration**                                                       | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
| ----------------------------------------------------------------------- | ---------------------------- | -------------------- | ---------------------------- | -------------------------- | ------------------------------------------- | ----------------------------------- | -------------------------------------- |
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)**               | N/A                          | 61.300 ms            | 0.0004 ms                    | 0.0070 ms                  | 0.0004 ms (0.0007%)                         | 61.3065 ms (99.999%)                | 61.307 ms                              |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 103.06 ms                    | 134.865 ms           | 0.0009 ms                    | 0.0101 ms                  | 103.061 ms (43.31%)                         | 134.875 ms (56.69%)                 | 237.936 ms (accounts for initial copy) |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)**  | 278.011 ms                   | 13.349 ms            | 308.122 ms                   | 0.0015 ms                  | 586.133 ms (97.77%)                         | 13.3506 ms (2.23%)                  | 599.483 ms                             |




### Analysis

1. **Metal - Direct CPU Write to Metal Buffer** (M1 Pro):
   - **Memory Transfer Efficiency**: Minimal memory transfer time (0.02% of total time), as there is no need for repeated CPU-GPU copies due to unified memory access.
   - **Compute Dominance**: Most of the time is spent on computation, leveraging Metal’s shared memory model for seamless access.
   - **Total Execution Time**: 0.413 ms per iteration, the fastest among the three configurations, demonstrating the benefit of unified memory.

2. **Metal - CPU Write to CPU Buffer, Initial Copy to Metal** (M1 Pro):
   - **One-Time Transfer Cost**: An initial CPU-to-GPU transfer of 0.0371 ms adds a modest, upfront overhead without slowing down GPU computation.
   - **Effective for Compute-Intensive Workloads**: After the initial transfer, memory transfer costs remain low (8.39% of total time), with 91.61% of time spent on computation.
   - **Total Execution Time**: 0.443 ms per iteration, slightly slower than direct Metal buffer writes but still much faster than CUDA with repeated transfers.

3. **CUDA - Repeated CPU-GPU Transfers** (RTX 4080 & Intel i9-13900K):
   - **High Transfer Overhead**: Memory transfer between CPU and GPU constitutes 90.22% of the total execution time, resulting in significant overhead.
   - **Compute Bottleneck**: Although the GPU computation is fast (0.07 ms), the repeated data transfer reduces overall efficiency.
   - **Total Execution Time**: 0.720 ms per iteration, the slowest configuration due to the recurring CPU-GPU memory copy requirement.

### Key Takeaways

- **Unified Memory Advantage with Metal**: Both Metal configurations outperform CUDA by eliminating or minimizing data transfer overhead. Direct writes to Metal's unified buffer (0.413 ms) demonstrate the lowest latency, showcasing the efficiency of Metal's shared memory model.
- **CUDA’s Bottleneck with Repeated Transfers**: CUDA’s need for CPU-GPU transfers each iteration significantly impacts performance, with 90% of the time spent on data transfers.

## CPU only reference

For reference, running the power computation and binary search solely on the CPU (with OpenMP) yields the following times for 1G elements:

### Intel i9-13900K

- **Average CPU power computation time**: 288.874 ms
- **Average CPU binary search time**: 0.0025022 ms
- **Total average execution time per iteration**: 288.877 ms
