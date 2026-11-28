#include <cmath>
#include <iostream>
#include "gemm.h"

static void gemm_golden(const float* input,
                        const float* weight_t,
                        const float* bias,
                        float* output,
                        int M, int N, int K, bool use_bias) {
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < N; ++j) {
            float acc = use_bias ? bias[j] : 0.0f;
            for (int k = 0; k < K; ++k) {
                acc += input[i * K + k] * weight_t[j * K + k];
            }
            output[i * N + j] = acc;
        }
    }
}

int main() {
    const int M = 16;
    const int K = 16;
    const int N = 16;
    const bool use_bias = true;

    float input[MAX_M * MAX_K];
    float weight_t[MAX_N * MAX_K];
    float bias[MAX_N];
    float hw_out[MAX_M * MAX_N] = {0.0f};
    float sw_out[MAX_M * MAX_N] = {0.0f};

    for (int i = 0; i < M * K; ++i) {
        input[i] = static_cast<float>((i % 13) - 6) * 0.1f;
    }
    for (int i = 0; i < N * K; ++i) {
        weight_t[i] = static_cast<float>((i % 11) - 5) * 0.1f;
    }
    for (int j = 0; j < N; ++j) {
        bias[j] = static_cast<float>((j % 7) - 3) * 0.05f;
    }

    gemm_golden(input, weight_t, bias, sw_out, M, N, K, use_bias);
    gemm_hls(input, weight_t, bias, hw_out, M, N, K, use_bias ? 1 : 0);

    float max_err = 0.0f;
    for (int i = 0; i < M * N; ++i) {
        float err = std::fabs(hw_out[i] - sw_out[i]);
        if (err > max_err) max_err = err;
    }

    std::cout << "Max error vs golden: " << max_err << "\n";
    if (max_err > 1e-4f) {
        std::cerr << "FAIL: mismatch exceeds tolerance!\n";
        return 1;
    }
    std::cout << "PASS: C simulation matches golden reference.\n";
    return 0;
}
