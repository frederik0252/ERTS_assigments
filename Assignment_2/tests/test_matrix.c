#include <stdio.h>
#include <stdlib.h>

#include "matrix.h"

int main(void)
{
    static const uint8_t expected[MATRIX_DIM][MATRIX_DIM] = {
        {10, 20, 30, 40},
        {26, 52, 78, 104},
        {42, 84, 126, 168},
        {58, 116, 174, 232}
    };
    unsigned row;
    unsigned col;

    setInputMatrices();
    multiMatrixSoft(aInst, bTInst, pInst);
    for (row = 0U; row < MATRIX_DIM; ++row) {
        for (col = 0U; col < MATRIX_DIM; ++col) {
            if (pInst[row].comp[col] != expected[row][col]) {
                fprintf(stderr, "Mismatch at [%u][%u]: %u != %u\n", row, col,
                        pInst[row].comp[col], expected[row][col]);
                return EXIT_FAILURE;
            }
        }
    }
    puts("matrix test: PASS");
    return EXIT_SUCCESS;
}

