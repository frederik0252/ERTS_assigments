#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <vector>
#include "xtime_l.h"

template <typename T>
inline void do_not_optimize(const T& val) {
    asm volatile("" : : "r,m"(val) : "memory");
}

// Mirrors PyTorch torch.nn.LayerNorm over last dim D:
//   input:  [M, D] row-major (e.g. num_tokens x embed_dim)
//   gamma:  [D] optional scale weights (pass nullptr if affine=false)
//   beta:   [D] optional bias weights (pass nullptr if affine=false)
//   output: [M, D] row-major
void layernorm_sw(const float* input,
                  const float* gamma,
                  const float* beta,
                  float* output,
                  int M, int D, float eps) {
    for (int i = 0; i < M; ++i) {
        const float* x_row = input + static_cast<size_t>(i) * D;
        float* y_row = output + static_cast<size_t>(i) * D;

        float sum = 0.0f;
        for (int d = 0; d < D; ++d) {
            sum += x_row[d];
        }
        const float mean = sum / static_cast<float>(D);

        float var_sum = 0.0f;
        for (int d = 0; d < D; ++d) {
            const float diff = x_row[d] - mean;
            var_sum += diff * diff;
        }
        const float inv_std = 1.0f / std::sqrt((var_sum / static_cast<float>(D)) + eps);

        for (int d = 0; d < D; ++d) {
            float norm = (x_row[d] - mean) * inv_std;
            if (gamma != nullptr) {
                norm *= gamma[d];
            }
            if (beta != nullptr) {
                norm += beta[d];
            }
            y_row[d] = norm;
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
    // Match RF-DETR Nano token count (M) and hidden/channel dimension (D)
    const int M = 256;
    const int D = 192;
    const int runs = 10;
    const bool affine = true;
    const float eps = 1e-5f;

    std::vector<float> input(static_cast<size_t>(M) * D);
    std::vector<float> gamma(affine ? static_cast<size_t>(D) : 0, 1.0f);
    std::vector<float> beta(affine ? static_cast<size_t>(D) : 0, 0.0f);
    std::vector<float> output(static_cast<size_t>(M) * D, 0.0f);

    for (size_t i = 0; i < input.size(); ++i) {
        input[i] = static_cast<float>(static_cast<int>(i % 97) - 48) * 0.02f;
    }
    for (size_t d = 0; d < gamma.size(); ++d) {
        gamma[d] = 1.0f + static_cast<float>(static_cast<int>(d % 23) - 11) * 0.01f;
        beta[d] = static_cast<float>(static_cast<int>(d % 19) - 9) * 0.005f;
    }

    const float* gamma_ptr = affine ? gamma.data() : nullptr;
    const float* beta_ptr = affine ? beta.data() : nullptr;

    // Warmup run (excluded from timing)
    layernorm_sw(input.data(), gamma_ptr, beta_ptr, output.data(), M, D, eps);
    do_not_optimize(output.front());
    do_not_optimize(output.back());

    std::vector<double> ln_times;
    ln_times.reserve(runs);

    for (int r = 0; r < runs; ++r) {
        XTime t0, t1;
        XTime_GetTime(&t0);
        layernorm_sw(input.data(), gamma_ptr, beta_ptr, output.data(), M, D, eps);
        XTime_GetTime(&t1);
        do_not_optimize(output.front());
        do_not_optimize(output.back());

        double ms = ticks_to_ms(t0, t1);
        ln_times.push_back(ms);
        std::cout << "LayerNorm run " << (r + 1) << ": "
                  << std::fixed << std::setprecision(3) << ms << " ms\n";
    }

    const size_t input_bytes  = input.size() * sizeof(float);
    const size_t output_bytes = output.size() * sizeof(float);
    const size_t param_bytes  = (gamma.size() + beta.size()) * sizeof(float);
    const size_t io_bytes     = input_bytes + output_bytes;
    const size_t total_bytes  = io_bytes + param_bytes;

    const uint64_t total_elems = static_cast<uint64_t>(M) * static_cast<uint64_t>(D);
    // Approximate elementwise/reduction arithmetic ops per layer call:
    // ~M * (D sum + D diff^2 + D norm + (affine ? 2*D : 0)) + ~4*M scalar ops
    const uint64_t approx_ops =
        total_elems * (affine ? 5ULL : 3ULL) + static_cast<uint64_t>(M) * 4ULL;

    const double med_ms = median_ms(ln_times);
    const double med_s  = med_ms / 1000.0;
    const double eff_bw_mbs = (med_s > 0.0)
        ? (static_cast<double>(total_bytes) / (1024.0 * 1024.0)) / med_s
        : 0.0;

    std::cout << "\nLayerNorm Summary (M=" << M << ", D=" << D
              << ", affine=" << (affine ? "true" : "false") << "):\n";
    std::cout << "  Median latency:             " << std::fixed << std::setprecision(3)
              << med_ms << " ms\n";
    std::cout << "  Approx arithmetic ops:      " << approx_ops << "\n";
    std::cout << "  Input buffer:               " << std::setprecision(2)
              << (input_bytes / 1024.0) << " KiB\n";
    std::cout << "  Output buffer:              " << (output_bytes / 1024.0) << " KiB\n";
    std::cout << "  Params (gamma+beta):        " << (param_bytes / 1024.0) << " KiB\n";
    std::cout << "  Per-frame I/O (in+out):     " << (io_bytes / 1024.0) << " KiB\n";
    std::cout << "  Total buffer footprint:     " << (total_bytes / 1024.0) << " KiB ("
              << total_bytes << " bytes)\n";
    std::cout << "  Effective buffer BW (SW):   " << eff_bw_mbs << " MiB/s\n";
    std::cout << "  Sample output[0]:           " << std::setprecision(6) << output[0] << "\n";

    return 0;
}
