#include "matrix.h"

#include <stddef.h>

vectorArray pInst[MATRIX_DIM];
vectorArray aInst[MATRIX_DIM];
vectorArray bTInst[MATRIX_DIM];

void setInputMatrices(void)
{
    static const uint8_t a[MATRIX_DIM][MATRIX_DIM] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12},
        {13, 14, 15, 16}
    };
    static const uint8_t bT[MATRIX_DIM][MATRIX_DIM] = {
        {1, 1, 1, 1},
        {2, 2, 2, 2},
        {3, 3, 3, 3},
        {4, 4, 4, 4}
    };
    size_t row;
    size_t col;

    for (row = 0; row < MATRIX_DIM; ++row) {
        for (col = 0; col < MATRIX_DIM; ++col) {
            aInst[row].comp[col] = a[row][col];
            bTInst[row].comp[col] = bT[row][col];
        }
    }
    clearProductMatrix();
}

void clearProductMatrix(void)
{
    size_t row;

    for (row = 0; row < MATRIX_DIM; ++row) {
        pInst[row].vect = 0U;
    }
}

void multiMatrixSoft(const vectorArray a[MATRIX_DIM],
                     const vectorArray bT[MATRIX_DIM],
                     vectorArray product[MATRIX_DIM])
{
    size_t row;
    size_t col;
    size_t k;

    for (row = 0; row < MATRIX_DIM; ++row) {
        for (col = 0; col < MATRIX_DIM; ++col) {
            uint32_t sum = 0U;
            for (k = 0; k < MATRIX_DIM; ++k) {
                sum += (uint32_t)a[row].comp[k] * bT[col].comp[k];
            }
            product[row].comp[col] = (uint8_t)sum;
        }
    }
}

int matrixEquals(const vectorArray lhs[MATRIX_DIM],
                 const vectorArray rhs[MATRIX_DIM])
{
    size_t row;

    for (row = 0; row < MATRIX_DIM; ++row) {
        if (lhs[row].vect != rhs[row].vect) {
            return 0;
        }
    }
    return 1;
}

