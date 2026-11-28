#include "gemm.h"

void gemm_hls(const float input[MAX_M * MAX_K],
              const float weight_t[MAX_N * MAX_K],
              const float bias[MAX_N],
              float output[MAX_M * MAX_N],
              int M, int N, int K, int use_bias) {
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL
#pragma HLS INTERFACE s_axilite port=M bundle=CTRL
#pragma HLS INTERFACE s_axilite port=N bundle=CTRL
#pragma HLS INTERFACE s_axilite port=K bundle=CTRL
#pragma HLS INTERFACE s_axilite port=use_bias bundle=CTRL
#pragma HLS INTERFACE s_axilite port=input bundle=CTRL
#pragma HLS INTERFACE s_axilite port=weight_t bundle=CTRL
#pragma HLS INTERFACE s_axilite port=bias bundle=CTRL
#pragma HLS INTERFACE s_axilite port=output bundle=CTRL

    for (int i = 0; i < M; ++i) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
        for (int j = 0; j < N; ++j) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
            float acc = (use_bias != 0) ? bias[j] : 0.0f;
            for (int k = 0; k < K; ++k) {
#pragma HLS LOOP_TRIPCOUNT min=1 max=64
                acc += input[i * K + k] * weight_t[j * K + k];
            }
            output[i * N + j] = acc;
        }
    }
}
