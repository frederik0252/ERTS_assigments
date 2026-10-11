#include "matrix.h"

#include <stddef.h>

VectorArray pInst;
VectorArray aInst;
VectorArray bTInst;

void setInputMatrices(VectorArray A, VectorArray B)
{
    static const unsigned char a[MSIZE][MSIZE] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    /* Literal transpose printed for bTInst on assignment page 6. */
    static const unsigned char bT[MSIZE][MSIZE] = {
        {1, 2, 3, 4},
        {1, 2, 3, 4},
        {1, 2, 3, 4},
        {1, 2, 3, 4}
    };
    size_t row;
    size_t col;

    for (row = 0; row < MSIZE; ++row) {
        for (col = 0; col < MSIZE; ++col) {
            A[row].comp[col] = a[row][col];
            B[row].comp[col] = bT[row][col];
        }
    }
}

void multiMatrixSoft(VectorArray A, VectorArray B, VectorArray P)
{
    size_t row;
    size_t col;
    size_t k;

    for (row = 0; row < MSIZE; ++row) {
        for (col = 0; col < MSIZE; ++col) {
            unsigned int sum = 0U;
            for (k = 0; k < MSIZE; ++k) {
                sum += (unsigned int)A[row].comp[k] * B[col].comp[k];
            }
            /* Same modulo-256 result as the supplied eight-bit dot-product IP. */
            P[row].comp[col] = (unsigned char)sum;
        }
    }
}

int matrixEquals(const vectorType lhs[MSIZE], const vectorType rhs[MSIZE])
{
    size_t row;

    for (row = 0; row < MSIZE; ++row) {
        if (lhs[row].vect != rhs[row].vect) {
            return 0;
        }
    }
    return 1;
}
