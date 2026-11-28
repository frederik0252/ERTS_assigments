#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include "xtime_l.h"

template <typename T>
inline void do_not_optimize(const T& val) {
    asm volatile("" : : "r,m"(val) : "memory");
}

// Mirrors PyTorch torch.nn.functional.linear(input, weight_t, bias):
//   input:    [M, K] row-major (e.g. num_tokens x in_features)
//   weight_t: [N, K] row-major (PyTorch nn.Linear.weight has shape [out_features=N, in_features=K])
//   bias:     [N] optional vector; pass nullptr if bias=None in Python
//   output:   [M, N] row-major
void linear_gemm_sw(const float* input,
                    const float* weight_t,
                    const float* bias,
                    float* output,
                    int M, int N, int K) {
    for (int i = 0; i < M; ++i) {
        const float* x_row = input + static_cast<size_t>(i) * K;
        float* y_row = output + static_cast<size_t>(i) * N;

        for (int j = 0; j < N; ++j) {
            const float* w_row = weight_t + static_cast<size_t>(j) * K;
            float acc = (bias != nullptr) ? bias[j] : 0.0f;

            for (int k = 0; k < K; ++k) {
                acc += x_row[k] * w_row[k];
            }
            y_row[j] = acc;
        }
    }
}

double ticks_to_ms(XTime start_t, XTime end_t) {
    return (static_cast<double>(end_t - start_t) * 1000.0) /
           static_cast<double>(COUNTS_PER_SECOND);
}

double median_ms(std::vector<double> times) {
    std::sort(times.begin(), times.end());
    size_t n = times.size();
    if (n % 2 == 0) {
        return 0.5 * (times[n / 2 - 1] + times[n / 2]);
    }
    return times[n / 2];
}

int main() {
    // Set M (tokens/rows), K (in_features), and N (out_features) to match
    // the specific Linear/MatMul layer you export from RF-DETR Nano.
    const int M = 256;
    const int K = 192;
    const int N = 192;
    const int runs = 10;
    const bool use_bias = true;

    std::vector<float> input(static_cast<size_t>(M) * K);
    std::vector<float> weight_t(static_cast<size_t>(N) * K);
    std::vector<float> bias(use_bias ? static_cast<size_t>(N) : 0, 0.0f);
    std::vector<float> output(static_cast<size_t>(M) * N, 0.0f);

    // Deterministic test tensors (replace with exported ONNX/PyTorch weights for exact layer checks)
    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = static_cast<float>(static_cast<int>(i % 97) - 48) * 0.02f;
    }
    for (size_t i = 0; i < weight_t.size(); ++i) {
        weight_t[i] = static_cast<float>(static_cast<int>(i % 89) - 44) * 0.015f;
    }
    for (size_t j = 0; j < bias.size(); ++j) {
        bias[j] = static_cast<float>(static_cast<int>(j % 31) - 15) * 0.005f;
    }

    const float* bias_ptr = use_bias ? bias.data() : nullptr;

    // Warmup run (excluded from timing)
    linear_gemm_sw(input.data(), weight_t.data(), bias_ptr, output.data(), M, N, K);
    do_not_optimize(output[0]);

    std::vector<double> gemm_times;
    gemm_times.reserve(runs);

    for (int r = 0; r < runs; ++r) {
        XTime t0, t1;
        XTime_GetTime(&t0);
        linear_gemm_sw(input.data(), weight_t.data(), bias_ptr, output.data(), M, N, K);
        XTime_GetTime(&t1);
        do_not_optimize(output[0]);

        double ms = ticks_to_ms(t0, t1);
        gemm_times.push_back(ms);
        std::cout << "GEMM run " << (r + 1) << ": "
                  << std::fixed << std::setprecision(3) << ms << " ms\n";
    }

    size_t buf_bytes = (input.size() + weight_t.size() + bias.size() + output.size()) *
                       sizeof(float);

    std::cout << "\nGEMM Summary (M=" << M << ", K=" << K << ", N=" << N
              << ", bias=" << (use_bias ? "true" : "false") << "):\n";
    std::cout << "  Median latency:           " << median_ms(gemm_times) << " ms\n";
    std::cout << "  Approx buffer footprint:  " << (buf_bytes / 1024.0) << " KiB\n";
    std::cout << "  Sample output[0]:         " << std::setprecision(6) << output[0] << "\n";



    size_t input_bytes  = input.size() * sizeof(float);
    size_t weight_bytes = (weight_t.size() + bias.size()) * sizeof(float);
    size_t output_bytes = output.size() * sizeof(float);
    size_t total_bytes  = input_bytes + weight_bytes + output_bytes;
    size_t per_frame_stream_bytes = input_bytes + output_bytes; // if weights stay on FPGA
    uint64_t macs = static_cast<uint64_t>(M) * N * K;

    std::cout << "  MACs:                     " << macs << "\n";
    std::cout << "  Total buffer footprint:   " << (total_bytes / 1024.0) << " KiB\n";
    std::cout << "  Per-frame I/O (w cached): " << (per_frame_stream_bytes / 1024.0) << " KiB\n";
    return 0;
}
