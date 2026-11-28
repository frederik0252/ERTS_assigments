#include <cmath>
#include <cstdlib>
#include <iostream>
#include "layernorm.h"

static void layernorm_golden(const float* input,
                             const float* gamma,
                             const float* beta,
                             float* output,
                             int num_tokens,
                             int dim,
                             float eps) {
    for (int t = 0; t < num_tokens; ++t) {
        int base = t * dim;
        double sum = 0.0;
        for (int d = 0; d < dim; ++d) {
            sum += static_cast<double>(input[base + d]);
        }
        double mean = sum / static_cast<double>(dim);

        double sq_sum = 0.0;
        for (int d = 0; d < dim; ++d) {
            double diff = static_cast<double>(input[base + d]) - mean;
            sq_sum += diff * diff;
        }
        double var = sq_sum / static_cast<double>(dim);
        double inv_std = 1.0 / std::sqrt(var + static_cast<double>(eps));

        for (int d = 0; d < dim; ++d) {
            double norm = (static_cast<double>(input[base + d]) - mean) * inv_std;
            output[base + d] = static_cast<float>(norm * static_cast<double>(gamma[d]) +
                                                  static_cast<double>(beta[d]));
        }
    }
}

int main() {
    const int num_tokens = 256;
    const int dim = 192;
    const float eps = 1e-5f;

    static float input[MAX_TOKENS * MAX_DIM];
    static float gamma[MAX_DIM];
    static float beta[MAX_DIM];
    static float hw_output[MAX_TOKENS * MAX_DIM];
    static float ref_output[MAX_TOKENS * MAX_DIM];

    std::srand(42);
    for (int i = 0; i < num_tokens * dim; ++i) {
        input[i] = (static_cast<float>(std::rand() % 2000) - 1000.0f) / 100.0f;
        hw_output[i] = 0.0f;
        ref_output[i] = 0.0f;
    }
    for (int d = 0; d < dim; ++d) {
        gamma[d] = 0.5f + static_cast<float>(std::rand() % 100) / 100.0f;
        beta[d] = -0.5f + static_cast<float>(std::rand() % 100) / 100.0f;
    }

    layernorm_golden(input, gamma, beta, ref_output, num_tokens, dim, eps);
    layernorm_hls(input, gamma, beta, hw_output, num_tokens, dim, eps);

    float max_err = 0.0f;
    for (int i = 0; i < num_tokens * dim; ++i) {
        float err = std::fabs(hw_output[i] - ref_output[i]);
        if (err > max_err) {
            max_err = err;
        }
    }

    std::cout << "Max error vs golden: " << max_err << std::endl;

    const float tol = 1e-3f;
    if (max_err > tol) {
        std::cerr << "FAIL: max error exceeded tolerance (" << tol << ")" << std::endl;
        return 1;
    }

    std::cout << "PASS: LayerNorm C simulation matched reference." << std::endl;
    return 0;
}
