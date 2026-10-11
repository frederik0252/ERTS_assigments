#include "matrix.h"

#include "xil_printf.h"

void displayMatrix(VectorArray input)
{
    unsigned int row;
    unsigned int col;

    for (row = 0U; row < MSIZE; ++row) {
        for (col = 0U; col < MSIZE; ++col) {
            xil_printf("%4d", input[row].comp[col]);
        }
        xil_printf("\r\n");
    }
}
