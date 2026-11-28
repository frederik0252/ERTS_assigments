#ifndef GEMM_H
#define GEMM_H

#define MAX_M 32
#define MAX_K 32
#define MAX_N 32

void gemm_hls(const float input[MAX_M * MAX_K],
              const float weight_t[MAX_N * MAX_K],
              const float bias[MAX_N],
              float output[MAX_M * MAX_N],
              int M, int N, int K, int use_bias);

#endif
