#ifndef ERTS_MATRIX_H
#define ERTS_MATRIX_H

#include <limits.h>

#define MSIZE 4

#if UCHAR_MAX != 255 || UINT_MAX != 0xFFFFFFFFU
#error "The packed matrix interface requires 8-bit bytes and 32-bit unsigned int"
#endif

/*
 * Course interface: four unsigned bytes packed in one 32-bit AXI register.
 * The assignment's hardware IP performs four independent 8-bit products and
 * adds the low byte from each product.
 */
typedef union {
    unsigned char comp[MSIZE];
    unsigned int vect;
} vectorType;

typedef vectorType VectorArray[MSIZE];

extern VectorArray pInst;
extern VectorArray aInst;
extern VectorArray bTInst;

void setInputMatrices(VectorArray A, VectorArray B);
void displayMatrix(VectorArray input);
void multiMatrixSoft(VectorArray A, VectorArray B, VectorArray P);
void multiMatrixHard(VectorArray A, VectorArray B, VectorArray P);
int matrixEquals(const vectorType lhs[MSIZE], const vectorType rhs[MSIZE]);

#endif
