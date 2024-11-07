# Metal POC
Demonstrate zero cost memory transfer between CPU and GPU in Metal (for Mac M socs)

# Metal vs CUDA POC: Analyzing Memory Transfer Bottlenecks in Mixed Workloads

## 1. POC Goals

This POC compares the performance impact of memory transfer bottlenecks between CPU and GPU in mixed computation workflows using CUDA and Metal. 

### Objectives
- **Highlight CUDA Limitations**: Demonstrate the bottleneck caused by memory transfers in CUDA when CPU computation is required as a fallback.
- **Show Metal’s unified Memory Advantage**: Emphasize the efficiency of Metal’s unified CPU-GPU memory architecture, which reduces or removes the need for costly memory transfers, providing an incentive for a Metal backend for applications like ICICLE.

## 2. POC Flows

### CUDA Flow
1. **Allocate Memory on CPU**: Initialize a sorted array of random numbers on the CPU.
2. **Copy Memory to GPU**: Transfer the sorted array to the GPU (measure time).
3. **Power Computation on GPU**: Raise each element to a power (e.g., square each element) while maintaining order (measure time).
4. **Copy Memory Back to CPU**: Transfer the modified array back to the CPU (measure time).
5. **Binary Search on CPU**: Perform a binary search on the powered array to check for the presence of a specific value (measure time).
6. **Repeat**: Execute this workflow multiple times to accumulate meaningful timing data.

### Metal Flow
1. **Allocate Memory and Initialize Data**: Initialize a sorted array of random numbers on unified memory accessible to both CPU and GPU.
2. **Power Computation on GPU**: Perform the same power operation directly on the unified memory.
3. **Binary Search on CPU**: Directly perform a binary search on the modified array in unified memory, eliminating the need for data copying.
4. **Repeat**: Iterate this flow to observe performance consistency.

## 3. Define Computations

### GPU Computation
   - **Task**: Elementwise power operation on each element in a large, sorted array (e.g., square each element).
   - **Rationale**: This is a straightforward, high-performance task for the GPU.

### CPU Computation
   - **Task**: Binary search for a specific value within the powered array.
   - **Rationale**: Binary search is inherently sequential and ideal for the CPU, illustrating the efficiency of CPU fallback for branching logic.

## 4. Metal unified Memory Research

Before implementing the flows, it’s essential to understand Metal’s memory management to gauge potential cost and performance trade-offs:
- **Zero-Cost unified Memory**: Determine if Metal’s unified memory model truly provides zero-cost access across CPU and GPU.
- **Memory Access Patterns and Costs**: Assess if Metal offers various memory tiers (e.g., faster but more costly memory options) and if data locality impacts performance.
- **Mechanics of unified Memory**: Review Metal’s documentation to understand how unified memory is allocated, accessed, and synchronized.

[Check the summary here](./metal-programming-notes.md)


## 5. Implementation Steps

### CUDA Flow
   - **Implement** the CPU-GPU CUDA flow.
   - **Benchmark** the time for each step: memory allocation, data transfer (to/from GPU), GPU computation, and CPU operation.
   - **Analyze** the results to confirm the high cost of memory transfers.

### Metal Flow
   - **Implement** the mixed CPU-GPU workflow using Metal, utilizing unified memory for data access across CPU and GPU.
   - **Benchmark** and compare with the CUDA results to observe reduced or eliminated transfer costs.
   - **Performance Comparison**: Assuming GPU computation may be slower on Metal than a high-end CUDA GPU (e.g., NVIDIA RTX 4080), check if transfer savings compensate for this difference.

## 6. Conclusion

| **Configuration**                         | **CPU to GPU Transfer Time** | **GPU Compute Time** | **GPU to CPU Transfer Time** | **CPU Binary Search Time** | **Total Memory Transfer Time (% of Total)** | **Total Compute Time (% of Total)** | **Total Execution Time per Iteration** |
|------------------------------------------|------------------------------|-----------------------|------------------------------|-----------------------------|-------------------------------------------|------------------------------------|---------------------------------------|
| **(1) Metal - Direct CPU Write to Metal Buffer (M1 Pro)** | N/A                         | 0.411 ms             | 9.58e-5 ms                  | 0.0018 ms                  | 9.58e-5 ms (0.02%)                       | 0.413 ms (99.98%)                  | 0.413 ms                             |
| **(2) Metal - CPU Write to CPU Buffer, Initial Copy to Metal (M1 Pro)** | 0.0371 ms                 | 0.405 ms             | 8.36e-5 ms                   | 0.0015 ms                  | 0.0372 ms (8.39%)                       | 0.406 ms (91.61%)                  | 0.443 ms (accounts for initial copy)                            |
| **(3) CUDA - Repeated CPU-GPU Transfers (RTX 4080 & Intel i9-13900K)**      | 0.296 ms                    | 0.070 ms             | 0.353 ms                    | 0.0006 ms                  | 0.649 ms (90.22%)                       | 0.070 ms (9.78%)                   | 0.720 ms                             |

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
