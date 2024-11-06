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

## 4. Metal unified Memory Research (**TODO**)

**TODO update here about Metal unified memory**

Before implementing the flows, it’s essential to understand Metal’s memory management to gauge potential cost and performance trade-offs:
- **Zero-Cost unified Memory**: Determine if Metal’s unified memory model truly provides zero-cost access across CPU and GPU.
- **Memory Access Patterns and Costs**: Assess if Metal offers various memory tiers (e.g., faster but more costly memory options) and if data locality impacts performance.
- **Mechanics of unified Memory**: Review Metal’s documentation to understand how unified memory is allocated, accessed, and synchronized.


## 5. Implementation Steps

### CUDA Flow
   - **Implement** the CPU-GPU CUDA flow.
   - **Benchmark** the time for each step: memory allocation, data transfer (to/from GPU), GPU computation, and CPU operation.
   - **Analyze** the results to confirm the high cost of memory transfers.

### Metal Flow
   - **Implement** the mixed CPU-GPU workflow using Metal, utilizing unified memory for data access across CPU and GPU.
   - **Benchmark** and compare with the CUDA results to observe reduced or eliminated transfer costs.
   - **Performance Comparison**: Assuming GPU computation may be slower on Metal than a high-end CUDA GPU (e.g., NVIDIA RTX 4080), check if transfer savings compensate for this difference.

## 6. Conclusion (**TODO**)

Document the findings in a final summary:
- **Performance Differences**: Clearly outline the measured transfer overhead in CUDA versus the efficiencies seen with Metal.
- **Incentive for a Metal Backend**: Summarize how Metal’s unified memory could reduce bottlenecks in scenarios requiring both CPU and GPU computations, supporting the case for developing a Metal backend for ICICLE.
