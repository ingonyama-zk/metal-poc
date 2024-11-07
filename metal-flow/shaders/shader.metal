#include <metal_stdlib>
using namespace metal;

struct Parameters {
    uint size;
    float power;
};

kernel void powerKernel(device float* data [[buffer(0)]],
                        constant Parameters& params [[buffer(1)]],
                        uint tid [[thread_position_in_grid]])
{
    if (tid < params.size) {
        data[tid] = pow(data[tid], params.power);
    }
}