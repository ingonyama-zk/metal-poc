#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Metal.h"

#include <iostream>
#include <algorithm>
#include <chrono>
#include <cassert>

// Toggle this define to switch between generating data on the CPU or directly on the Metal buffer
#define GENERATE_ON_METAL_BUFFER

// Define parameters struct to match shader struct
struct Parameters {
  uint32_t size;
  float power;
};

int main(int argc, char* argv[])
{
  // Check if log size is provided as a command-line argument
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <log_size (max 30)>" << std::endl;
    return 1;
  }

  // Parse log size from command line
  int logSize = std::atoi(argv[1]);
  assert(logSize <= 30 && "logSize must be 30 or less"); // Ensure logSize is no more than 30
  int arraySize = 1 << logSize;                          // Calculate the array size as 2^logSize

  // Other parameters
  const float power = 2.0f;          // Square each element
  const float searchValue = 1024.0f; // Value to search after power operation
  const int N = 10;                  // Number of repetitions

  // Create autorelease pool because we are using Objective-C objects (some macOS versions may not require this)
  NS::AutoreleasePool* p_pool = NS::AutoreleasePool::alloc()->init();

  // Initialize Metal device and command queue
  MTL::Device* device = MTL::CreateSystemDefaultDevice();
  if (!device) {
    std::cerr << "Metal is not supported on this device." << std::endl;
    return -1;
  }
  MTL::CommandQueue* cmdQueue = device->newCommandQueue();

  // Load Metal library and create compute pipeline state
  NS::Error* error = nullptr;
  NS::String* libPath = NS::String::string(METAL_LIBRARY_PATH, NS::StringEncoding::UTF8StringEncoding);
  MTL::Library* library = device->newLibrary(libPath, &error);
  if (error) {
    std::cerr << "Failed to create library: "
              << error->localizedDescription()->cString(NS::StringEncoding::UTF8StringEncoding) << std::endl;
    return -1;
  }
  MTL::Function* function =
    library->newFunction(NS::String::string("powerKernel", NS::StringEncoding::UTF8StringEncoding));
  if (!function) {
    std::cerr << "Failed to create function from library." << std::endl;
    return -1;
  }
  MTL::ComputePipelineState* pipelineState = device->newComputePipelineState(function, &error);
  if (error) {
    std::cerr << "Failed to create pipeline state: "
              << error->localizedDescription()->cString(NS::StringEncoding::UTF8StringEncoding) << std::endl;
    return -1;
  }

  // Create Metal buffer for data
  MTL::Buffer* bufferData = device->newBuffer(arraySize * sizeof(float), MTL::ResourceStorageModeShared);

  // Generate data either on the CPU or directly on the Metal buffer
  float totalCpuToGpuTime = 0.0f;

#ifdef GENERATE_ON_METAL_BUFFER
  // Generate data directly on the Metal buffer
  float* gpuData = static_cast<float*>(bufferData->contents());
  for (int i = 0; i < arraySize; ++i) {
    gpuData[i] = i;
  }
  std::cout << "Data generated directly on Metal buffer: not measuring this\n";
#else
  // Generate data on CPU and then copy to Metal buffer
  int i = 0;
  std::vector<float> hostData(arraySize);
  std::generate(hostData.begin(), hostData.end(), [&i]() { return i++; });

  // Simulate "transfer" to GPU by copying to shared memory
  auto start = std::chrono::high_resolution_clock::now();
  std::memcpy(bufferData->contents(), hostData.data(), arraySize * sizeof(float));
  auto end = std::chrono::high_resolution_clock::now();
  totalCpuToGpuTime = std::chrono::duration<float, std::milli>(end - start).count();
  std::cout << "CPU to GPU transfer time: " << totalCpuToGpuTime << " ms\n";
#endif

  // Create a constant buffer for parameters
  Parameters params = {static_cast<uint32_t>(arraySize), power};
  MTL::Buffer* paramsBuffer = device->newBuffer(&params, sizeof(Parameters), MTL::ResourceStorageModeShared);

  // Timing variables for accumulation
  float totalGpuComputeTime = 0.0f;
  float totalGpuToCpuTime = 0.0f;
  float totalCpuComputeTime = 0.0f;

  for (int i = 0; i <= N; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    // Launch GPU kernel
    MTL::CommandBuffer* commandBuffer = cmdQueue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = commandBuffer->computeCommandEncoder();
    encoder->setComputePipelineState(pipelineState);
    encoder->setBuffer(bufferData, 0, 0);
    encoder->setBuffer(paramsBuffer, 0, 1); // Pass parameters

    // Dispatch threads
    // NOTE: gridSize is the total number of threads, unlike CUDA where it defines number of thread-blocks
    MTL::Size gridSize = MTL::Size(arraySize, 1, 1);  // Set gridSize to the total number of elements in the array
    MTL::Size threadGroupSize = MTL::Size(256, 1, 1); // Set threadGroupSize to the number of threads per thread group
    encoder->dispatchThreads(gridSize, threadGroupSize);
    encoder->endEncoding();
    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
    auto end = std::chrono::high_resolution_clock::now();
    float gpuComputeTime = std::chrono::duration<float, std::milli>(end - start).count();
    if (i > 0) totalGpuComputeTime += gpuComputeTime;
    std::cout << "Iteration " << i + 1 << " - GPU computation time: " << gpuComputeTime << " ms\n";

    // Simulate "transfer" back to CPU by accessing shared memory directly
    start = std::chrono::high_resolution_clock::now();
    float* gpuResultData = static_cast<float*>(bufferData->contents());
    end = std::chrono::high_resolution_clock::now();
    float gpuToCpuTime = std::chrono::duration<float, std::milli>(end - start).count();
    if (i > 0) totalGpuToCpuTime += gpuToCpuTime;
    std::cout << "Iteration " << i + 1 << " - GPU to CPU transfer time: " << gpuToCpuTime << " ms\n";

    // Perform binary search on GPU data directly
    start = std::chrono::high_resolution_clock::now();
    bool found = std::binary_search(gpuResultData, gpuResultData + arraySize, searchValue);
    end = std::chrono::high_resolution_clock::now();
    float cpuComputeTime = std::chrono::duration<float, std::milli>(end - start).count();
    if (i > 0) totalCpuComputeTime += cpuComputeTime;
    std::cout << "Iteration " << i + 1 << " - CPU binary search time: " << cpuComputeTime << " ms\n";
    std::cout << "Iteration " << i + 1 << " - Value " << (found ? "found" : "not found") << " in the array.\n";
  }

  // Calculate and display average times
  float avgCpuToGpuTime = totalCpuToGpuTime / N;
  float avgGpuComputeTime = totalGpuComputeTime / N;
  float avgGpuToCpuTime = totalGpuToCpuTime / N;
  float avgCpuComputeTime = totalCpuComputeTime / N;

  std::cout << "\n=== Performance Summary ===\n";
#ifndef GENERATE_ON_METAL_BUFFER
  std::cout << "Initial CPU to GPU transfer time: " << avgCpuToGpuTime << " ms\n";
#endif
  std::cout << "Average GPU computation time: " << avgGpuComputeTime << " ms\n";
  std::cout << "Average GPU to CPU transfer time: " << avgGpuToCpuTime << " ms\n";
  std::cout << "Average CPU binary search time: " << avgCpuComputeTime << " ms\n";

  float avgTotalTransferTime = avgCpuToGpuTime + avgGpuToCpuTime;
  float avgTotalComputeTime = avgGpuComputeTime + avgCpuComputeTime;
  float totalAvgTime = avgTotalTransferTime + avgTotalComputeTime;

  std::cout << "Total average memory transfer time (CPU ↔ GPU): " << avgTotalTransferTime << " ms ("
            << (avgTotalTransferTime / totalAvgTime * 100) << "% of total time)\n";
  std::cout << "Total average compute time (GPU + CPU): " << avgTotalComputeTime << " ms ("
            << (avgTotalComputeTime / totalAvgTime * 100) << "% of total time)\n";
  std::cout << "Total average execution time per iteration: " << totalAvgTime << " ms\n";

  // Cleanup
  bufferData->release();
  paramsBuffer->release();
  pipelineState->release();
  function->release();
  library->release();
  cmdQueue->release();
  device->release();
  p_pool->release();

  return 0;
}