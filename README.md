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

| **Configuration**                                              | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
|----------------------------------------------------------------|------------------------------|-----------------------|------------------------------|-----------------------------|---------------------------------------------|--------------------------------------|---------------------------------------|
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)**      | N/A                          | 0.312 ms             | 0.0002 ms                    | 0.0030 ms                   | 0.0002 ms (0.06%)                          | 0.3152 ms (99.94%)                  | 0.3154 ms                             |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 0.0403 ms               | 0.276 ms             | 7.06e-5 ms                   | 0.0030 ms                   | 0.0404 ms (12.65%)                        | 0.2790 ms (87.35%)                  | 0.3194 ms (accounts for initial copy)   |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)** | 0.275 ms                   | 0.0674 ms           | 0.350 ms                     | 0.0003 ms                   | 0.6250 ms (90.22%)                        | 0.0677 ms (9.78%)                   | 0.6927 ms                             |


### Summary of Results for `log_size = 25`: Metal vs. CUDA

| **Configuration**                                              | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
|----------------------------------------------------------------|------------------------------|-----------------------|------------------------------|-----------------------------|---------------------------------------------|--------------------------------------|---------------------------------------|
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)**      | N/A                          | 2.215 ms             | 0.0004 ms                    | 0.0086 ms                   | 0.0004 ms (0.02%)                          | 2.2237 ms (99.98%)                  | 2.2241 ms                             |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 1.193 ms               | 2.011 ms             | 0.0003 ms                    | 0.0031 ms                   | 1.1931 ms (37.20%)                        | 2.0146 ms (62.80%)                  | 3.2077 ms (accounts for initial copy)   |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)** | 8.744 ms                   | 0.412 ms            | 9.620 ms                     | 0.0005 ms                   | 18.3643 ms (97.80%)                       | 0.4124 ms (2.20%)                   | 18.7767 ms                             |


### Summary of Results for `log_size = 30`: Metal vs. CUDA

| **Configuration**                                              | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
|----------------------------------------------------------------|------------------------------|-----------------------|------------------------------|-----------------------------|---------------------------------------------|--------------------------------------|---------------------------------------|
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)**      | N/A                          | 46.545 ms            | 0.0004 ms                    | 0.0059 ms                   | 0.0004 ms (0.0009%)                        | 46.5509 ms (99.9991%)               | 46.5513 ms                             |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 107.549 ms             | 46.456 ms           | 0.0004 ms                    | 0.0056 ms                   | 107.55 ms (69.83%)                        | 46.4618 ms (30.17%)                 | 154.012 ms (accounts for initial copy)   |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)** | 279.670 ms                | 13.357 ms           | 306.174 ms                   | 0.0011 ms                   | 585.844 ms (97.77%)                       | 13.3578 ms (2.23%)                  | 599.202 ms                             |



### Analysis

#### Metal - Direct CPU Write to Metal Buffer (M1 Pro)
- **Memory Transfer Efficiency**: This configuration achieves almost negligible memory transfer time (less than 0.02% of total time), thanks to Metal's unified memory model, which eliminates the need for repeated CPU-GPU data transfers.
- **Compute Dominance**: The majority of the execution time is spent on GPU computation, demonstrating the efficiency of Metal’s unified memory for direct access across CPU and GPU.
- **Total Execution Time**: This configuration consistently performs the fastest across all tested sizes, making it ideal for applications that frequently exchange data between CPU and GPU without the need for explicit data copying.

#### Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)
- **One-Time Transfer Cost**: An initial CPU-to-GPU data transfer introduces an upfront cost that varies with data size (e.g., 0.0371 ms for `log_size=20`, 1.193 ms for `log_size=25`, and 107.549 ms for `log_size=30`). However, after this one-time transfer, memory transfer costs remain very low, and no further CPU-GPU data copying is needed.
- **Effective for Compute-Intensive Workloads**: With the initial transfer complete, a large percentage of execution time is dedicated to computation, as the GPU directly accesses data in Metal’s unified memory.
- **Total Execution Time**: This setup offers high efficiency for compute-heavy tasks, with total times that are competitive but slightly higher than the direct Metal buffer write approach.

#### CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)
- **High Transfer Overhead**: This configuration incurs a substantial performance penalty from repeated CPU-GPU transfers every iteration. Memory transfer constitutes around 90% of total execution time, which significantly impacts overall performance.
- **Compute Bottleneck**: Although the GPU computation itself is fast, the recurring data transfer reduces overall efficiency.
- **Total Execution Time**: This setup has the slowest total execution time across all tested sizes due to the heavy memory transfer overhead, making it less ideal for workflows that require frequent CPU-GPU data exchanges.

---

### Key Takeaways
- **Unified Memory Advantage with Metal**: Both Metal configurations outperform CUDA by eliminating or minimizing data transfer overhead. Direct writes to Metal's unified buffer show the lowest latency, underscoring the efficiency of Metal's shared memory model.
- **CUDA’s Bottleneck with Repeated Transfers**: The need for repeated CPU-GPU transfers in CUDA significantly impacts performance, with about 90% of total execution time spent on data transfers, highlighting a clear advantage of Metal’s unified memory for mixed CPU-GPU workflows.

## CPU only reference

For reference, running the power computation and binary search solely on the CPU (with OpenMP) yields the following times for `log_size = 30` elements:

### Intel i9-13900K

- **Average CPU power computation time**: 288.874 ms
- **Average CPU binary search time**: 0.0025022 ms
- **Total average execution time per iteration**: 288.877 ms

> **Note:** For small size, openMP is not effective, therefore only measuring the case of `log_size = 30`.
