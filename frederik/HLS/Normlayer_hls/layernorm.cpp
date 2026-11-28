#include <cmath>
#include "layernorm.h"

void layernorm_hls(const float input[MAX_TOKENS * MAX_DIM],
                   const float gamma[MAX_DIM],
                   const float beta[MAX_DIM],
                   float output[MAX_TOKENS * MAX_DIM],
                   int num_tokens, int dim, float eps) {
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL
#pragma HLS INTERFACE s_axilite port=num_tokens bundle=CTRL
#pragma HLS INTERFACE s_axilite port=dim bundle=CTRL
#pragma HLS INTERFACE s_axilite port=eps bundle=CTRL
#pragma HLS INTERFACE s_axilite port=input bundle=CTRL
#pragma HLS INTERFACE s_axilite port=gamma bundle=CTRL
#pragma HLS INTERFACE s_axilite port=beta bundle=CTRL
#pragma HLS INTERFACE s_axilite port=output bundle=CTRL

    for (int t = 0; t < num_tokens; ++t) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=256
        int base = t * dim;
        float sum = 0.0f;
        for (int d = 0; d < dim; ++d) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=192
            sum += input[base + d];
        }
        float mean = sum / static_cast<float>(dim);

        float sq_sum = 0.0f;
        for (int d = 0; d < dim; ++d) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=192
            float diff = input[base + d] - mean;
            sq_sum += diff * diff;
        }
        float var = sq_sum / static_cast<float>(dim);
        float inv_std = 1.0f / std::sqrt(var + eps);

        for (int d = 0; d < dim; ++d) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=192
            float norm = (input[base + d] - mean) * inv_std;
            output[base + d] = norm * gamma[d] + beta[d];
        }
    }
}
