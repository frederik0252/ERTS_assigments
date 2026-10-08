#ifndef ERTS_MATRIX_H
#define ERTS_MATRIX_H

#include <stdint.h>

#define MATRIX_DIM 4

/*
 * Four signed 8-bit matrix elements packed in one 32-bit AXI register.
 * The assignment's hardware IP performs four independent 8-bit products and
 * adds the low byte from each product.
 */
typedef union {
    uint32_t vect;
    uint8_t comp[MATRIX_DIM];
} vectorArray;

extern vectorArray pInst[MATRIX_DIM];
extern vectorArray aInst[MATRIX_DIM];
extern vectorArray bTInst[MATRIX_DIM];

void setInputMatrices(void);
void clearProductMatrix(void);
void multiMatrixSoft(const vectorArray a[MATRIX_DIM],
                     const vectorArray bT[MATRIX_DIM],
                     vectorArray product[MATRIX_DIM]);
int matrixEquals(const vectorArray lhs[MATRIX_DIM],
                 const vectorArray rhs[MATRIX_DIM]);

#endif

