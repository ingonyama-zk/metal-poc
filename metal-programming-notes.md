
## Guides

- Best Practice Guide: [Metal Best Practices](https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/index.html#//apple_ref/doc/uid/TP40016642-CH27-SW1)
- Programming Guide: [Metal Programming Guide](https://developer.apple.com/library/archive/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/Introduction/Introduction.html#//apple_ref/doc/uid/TP40014221)
- Metal Shading Language Guide: [Metal Shading Language Specification](https://developer.apple.com/metal/Metal-Shading-Language-Specification.pdf)
- Sample Code: [Metal Sample Code](https://developer.apple.com/metal/sample-code/)
- Metal C++ API: [Metal C++](https://developer.apple.com/metal/cpp/)

## Unified Memory

Apple Silicon uses a unified memory architecture, meaning CPU, GPU, and other components access a single pool of memory, which removes the need for explicit memory transfers.

## Caching and Memory Coherency

Apple Silicon is designed to be cache-coherent across all cores, meaning the CPU and GPU can access and update shared data in memory without needing explicit synchronization by the developer. This allows for a more seamless interaction where, for example, the CPU can directly read or modify data written by the GPU.

## Synchronization and Memory Access Patterns

Apple provides API-level support in Metal for synchronizing data access between the CPU and GPU. While unified memory reduces the need for traditional locking mechanisms, the architecture does involve implicit synchronization costs when both CPU and GPU access the same data within a short timeframe.

**Note**: This is like a CUDA stream synchronization, waiting for tasks to complete and not due to cache coherency.

## Is It Zero Cost?

While it’s not completely “zero-cost,” Apple’s architecture significantly reduces costs compared to traditional memory management. Data access and synchronization are fast, but not entirely free; certain workloads may still experience mild latency or bottlenecks, especially if they require frequent switching of access between CPU and GPU.

### Shared Buffer

It’s possible to allocate a device buffer and let the CPU use it as well:

```cpp
// 1. Initialize Metal
MTL::Device* device = MTL::CreateSystemDefaultDevice();
if (!device) {
    std::cerr << "Metal is not supported on this device." << std::endl;
    return -1;
}

// 2. Create a shared buffer accessible by both CPU and GPU
const int arraySize = 1024;
const int bufferSize = arraySize * sizeof(float);
MTL::Buffer* buffer = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);

// 3. Initialize data on the CPU
float* data = static_cast<float*>(buffer->contents());
for (int i = 0; i < arraySize; ++i) {
    data[i] = static_cast<float>(i);
}

// Use a command queue to dispatch kernel with this buffer
MTL::CommandQueue* commandQueue = device->newCommandQueue();
```

## Memory

- Shared memory (CPU-GPU)
- Private GPU memory
- Managed memory (mapped to both GPU and CPU)

### Memory Hierarchy

- **Unified RAM** accessible to both CPU and GPU with a unified address space.
- **GPU Caches**:
  - L1 cache per GPU core
  - L2 cache shared across all GPU cores
- **CPU Caches**:
  - L1 cache per CPU core
  - L2 cache per CPU cluster (likely separate for P cores and E cores)
- **System Cache**:
  - L3 cache (system cache) shared between CPU and GPU
- **Cache Coherency**:
  - Managed in hardware; no need for explicit synchronization.
  - Hardware ensures consistency.

**Threadgroup memory** is equivalent to software-managed CUDA shared memory (where a block of threads shares memory managed by software).

**Note**:
- **Coherency**: Ensures all caches reflect the latest data for each memory location individually (they all see the same “truth” for each variable).
- **Consistency**: Ensures that the order of writes across multiple locations appears the same to all cores, preserving program order across the system.

## Programming Model

Similar to CUDA, Metal has **threadgroups** and **threads**:

- **Threadgroup**: Equivalent to a CUDA block.
  - Can share threadgroup memory.
  - Can synchronize with each other within the group (intra-group sync).
  - No inter-group synchronization.

- **Thread**: Equivalent to a CUDA thread.
  - Has local memory (registers).
  - Can access threadgroup memory.
  - Has a thread ID like CUDA.

Example of kernel dispatch:

```cpp
// Set up kernel launch parameters
MTL::Size gridSize = MTL::Size(arrayWidth, arrayHeight, 1);         // Total number of threads
MTL::Size threadgroupSize = MTL::Size(16, 16, 1);                   // Threads per threadgroup (e.g., 16x16)

// Launch kernel
encoder->dispatchThreads(gridSize, threadgroupSize);
```

Inside the kernel, thread and threadgroup IDs are accessible:

```metal
#include <metal_stdlib>
using namespace metal;

kernel void myKernel(device float* data [[buffer(0)]],
                     uint2 gid [[thread_position_in_grid]],
                     uint2 tid [[thread_position_in_threadgroup]]) {
    // gid is the unique thread ID across the entire grid
    // tid is the unique thread ID within the threadgroup
    data[gid.y * gridWidth + gid.x] += 1.0;
}
```

Shared memory is defined inside the kernel as:

```metal
#include <metal_stdlib>
using namespace metal;

kernel void sumReductionKernel(device float* input [[buffer(0)]],
                               device float* output [[buffer(1)]],
                               uint threadgroup_size,
                               uint2 gid [[thread_position_in_grid]],
                               uint tid [[thread_position_in_threadgroup]]) {

    // 1. Declare threadgroup memory
    threadgroup float sharedMemory[256];  // Static size, known at compile time

    // Alternatively, you could pass in the size dynamically as a parameter
    // threadgroup float sharedMemory[threadgroup_size];

    // 2. Load data into threadgroup memory
    sharedMemory[tid] = input[gid.x];
    threadgroup_barrier(mem_flags::mem_threadgroup);  // Synchronize all threads

    // 3. Perform a reduction within the threadgroup
    for (uint stride = threadgroup_size / 2; stride > 0; stride /= 2) {
        if (tid < stride) {
            sharedMemory[tid] += sharedMemory[tid + stride];
        }
        threadgroup_barrier(mem_flags::mem_threadgroup);  // Synchronize again
    }

    // 4. Write the result of the reduction for this threadgroup
    if (tid == 0) {
        output[gid.x / threadgroup_size] = sharedMemory[0];
    }
}
```

Typical dispatch (similar to CUDA):

```cpp
// Define total grid size and threadgroup size
MTL::Size gridSize = MTL::Size(totalThreads, 1, 1);        // Total number of threads
MTL::Size threadgroupSize = MTL::Size(256, 1, 1);          // Number of threads per threadgroup

// Dispatch kernel
encoder->setComputePipelineState(pipelineState);
encoder->setBuffer(inputBuffer, 0, 0);
encoder->setBuffer(outputBuffer, 0, 1);
encoder->dispatchThreads(gridSize, threadgroupSize);
```

## Open Questions

1. **Can I take an already allocated memory and use it with the GPU? If not, should I copy it to a GPU buffer?**
    - Answer: You cannot directly use host memory. You must allocate a buffer via the Metal API to ensure alignment and synchronization.
    - You can copy from host memory to a Metal buffer using `std::copy` (or `memcpy`).
    - Similar to CUDA, we can handle host memory and device memory separately.
    - **Idea**: Expose mapped memory by using a buffer as mapped memory, enabling both CPU and GPU access. CUDA has managed memory, but this approach may not be ideal.

2. **Is a `commandBuffer` equivalent to a CUDA stream, or is it a `commandQueue`?**
    - It appears that `commandQueue` is more similar to a CUDA stream, but synchronization can occur at the `commandBuffer` level.
    - For `Icicle`, use a `commandQueue` as a stream, synchronizing on the last `commandBuffer`.

3. **Do we need separate `commandQueue` per thread, etc.? How should we manage the default queue? (Maybe `thread_local`?)**

4. **Is managed memory interesting or relevant outside Apple Silicon?**
