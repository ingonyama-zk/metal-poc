#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

// Function to raise each element to a power (CPU-only version)
void powerComputationCPU(std::vector<float>& data, float power)
{
  for (auto& value : data) {
    value = std::pow(value, power);
  }
}

int main()
{
  // Parameters
  const int arraySize = 1 << 20;     // 1 million elements
  const float power = 2.0f;          // Square each element
  const float searchValue = 1024.0f; // Value to search after power operation
  const int N = 10;                  // Number of repetitions

  // Allocate and initialize data on CPU
  std::vector<float> hostData(arraySize);
  int i = 0;
  std::generate(hostData.begin(), hostData.end(), [&i]() { return static_cast<float>(i++); });
  std::sort(hostData.begin(), hostData.end());

  // Timing variables for accumulation
  float totalCpuComputeTime = 0.0f;
  float totalCpuBinarySearchTime = 0.0f;

  for (int i = 0; i < N; ++i) {
    // Perform power computation on the CPU
    auto start = std::chrono::high_resolution_clock::now();
    powerComputationCPU(hostData, power);
    auto end = std::chrono::high_resolution_clock::now();
    float cpuComputeTime = std::chrono::duration<float, std::milli>(end - start).count();
    totalCpuComputeTime += cpuComputeTime;
    std::cout << "Iteration " << i + 1 << " - CPU power computation time: " << cpuComputeTime << " ms\n";

    // Perform binary search on CPU
    start = std::chrono::high_resolution_clock::now();
    bool found = std::binary_search(hostData.begin(), hostData.end(), searchValue);
    end = std::chrono::high_resolution_clock::now();
    float cpuBinarySearchTime = std::chrono::duration<float, std::milli>(end - start).count();
    totalCpuBinarySearchTime += cpuBinarySearchTime;
    std::cout << "Iteration " << i + 1 << " - CPU binary search time: " << cpuBinarySearchTime << " ms\n";
    std::cout << "Iteration " << i + 1 << " - Value " << (found ? "found" : "not found") << " in the array.\n";
  }

  // Calculate average times
  float avgCpuComputeTime = totalCpuComputeTime / N;
  float avgCpuBinarySearchTime = totalCpuBinarySearchTime / N;
  float totalAvgTime = avgCpuComputeTime + avgCpuBinarySearchTime;

  // Display the average times and compute transfer/computation overheads
  std::cout << "\n=== Performance Summary ===\n";
  std::cout << "Average CPU power computation time: " << avgCpuComputeTime << " ms\n";
  std::cout << "Average CPU binary search time: " << avgCpuBinarySearchTime << " ms\n";
  std::cout << "Total average execution time per iteration: " << totalAvgTime << " ms\n";

  return 0;
}