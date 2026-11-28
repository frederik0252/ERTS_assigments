#ifndef LAYERNORM_H
#define LAYERNORM_H

#define MAX_TOKENS 256
#define MAX_DIM    192

void layernorm_hls(const float input[MAX_TOKENS * MAX_DIM],
                   const float gamma[MAX_DIM],
                   const float beta[MAX_DIM],
                   float output[MAX_TOKENS * MAX_DIM],
                   int num_tokens, int dim, float eps);

#endif
