#include <algorithm>
#include <chrono>
#include <cuda_runtime.h>
#include <iostream>
#include <vector>

// Kernel to raise each element to a power (e.g., square each element)
__global__ void powerKernel(float* data, int size, float power)
{
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < size) { data[idx] = powf(data[idx], power); }
}

// Function to check CUDA error
void checkCudaError(cudaError_t err, const char* msg)
{
  if (err != cudaSuccess) {
    std::cerr << "Error: " << msg << " (" << cudaGetErrorString(err) << ")" << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main()
{
  // Parameters
  const int arraySize = 1 << 25;
  const float power = 2.0f;          // Square each element
  const float searchValue = 1024.0f; // Value to search after power operation
  const int N = 10;                  // Number of repetitions

  // Allocate and initialize data on CPU
  int i = 0;
  std::vector<float> hostData(arraySize);
  std::generate(hostData.begin(), hostData.end(), [&i]() { return i++; });

  // Allocate memory on GPU
  float* deviceData;
  checkCudaError(cudaMalloc(&deviceData, arraySize * sizeof(float)), "Failed to allocate device memory");

  // Timing variables for accumulation
  float totalCpuToGpuTime = 0.0f;
  float totalGpuComputeTime = 0.0f;
  float totalGpuToCpuTime = 0.0f;
  float totalCpuComputeTime = 0.0f;

  for (int i = 0; i < N; ++i) {
    // Copy data to GPU
    auto start = std::chrono::high_resolution_clock::now();
    checkCudaError(
      cudaMemcpy(deviceData, hostData.data(), arraySize * sizeof(float), cudaMemcpyHostToDevice),
      "Failed to copy data to GPU");
    auto end = std::chrono::high_resolution_clock::now();
    float cpuToGpuTime = std::chrono::duration<float, std::milli>(end - start).count();
    totalCpuToGpuTime += cpuToGpuTime;
    std::cout << "Iteration " << i + 1 << " - CPU to GPU transfer time: " << cpuToGpuTime << " ms\n";

    // Launch kernel to perform power operation
    int threadsPerBlock = 256;
    int blocksPerGrid = (arraySize + threadsPerBlock - 1) / threadsPerBlock;
    start = std::chrono::high_resolution_clock::now();
    powerKernel<<<blocksPerGrid, threadsPerBlock>>>(deviceData, arraySize, power);
    checkCudaError(cudaDeviceSynchronize(), "Kernel execution failed");
    end = std::chrono::high_resolution_clock::now();
    float gpuComputeTime = std::chrono::duration<float, std::milli>(end - start).count();
    totalGpuComputeTime += gpuComputeTime;
    std::cout << "Iteration " << i + 1 << " - GPU computation time: " << gpuComputeTime << " ms\n";

    // Copy result back to CPU
    start = std::chrono::high_resolution_clock::now();
    checkCudaError(
      cudaMemcpy(hostData.data(), deviceData, arraySize * sizeof(float), cudaMemcpyDeviceToHost),
      "Failed to copy data to CPU");
    end = std::chrono::high_resolution_clock::now();
    float gpuToCpuTime = std::chrono::duration<float, std::milli>(end - start).count();
    totalGpuToCpuTime += gpuToCpuTime;
    std::cout << "Iteration " << i + 1 << " - GPU to CPU transfer time: " << gpuToCpuTime << " ms\n";

    // Perform binary search on CPU
    start = std::chrono::high_resolution_clock::now();
    bool found = std::binary_search(hostData.begin(), hostData.end(), searchValue);
    end = std::chrono::high_resolution_clock::now();
    float cpuComputeTime = std::chrono::duration<float, std::milli>(end - start).count();
    totalCpuComputeTime += cpuComputeTime;
    std::cout << "Iteration " << i + 1 << " - CPU binary search time: " << cpuComputeTime << " ms\n";
    std::cout << "Iteration " << i + 1 << " - Value " << (found ? "found" : "not found") << " in the array.\n";
  }

  // Calculate average times
  float avgCpuToGpuTime = totalCpuToGpuTime / N;
  float avgGpuComputeTime = totalGpuComputeTime / N;
  float avgGpuToCpuTime = totalGpuToCpuTime / N;
  float avgCpuComputeTime = totalCpuComputeTime / N;

  // Display the average times and compute transfer/computation overheads
  std::cout << "\n=== Performance Summary ===\n";
  std::cout << "Average CPU to GPU transfer time: " << avgCpuToGpuTime << " ms\n";
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
  checkCudaError(cudaFree(deviceData), "Failed to free device memory");

  return 0;
}