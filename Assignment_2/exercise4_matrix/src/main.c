#include <string.h>

#include "platform.h"
#include "xstatus.h"

#include "console.h"
#include "matrix.h"
#include "zynq_timer.h"

static ErtsTimer timer;

static void displayMatrix(const char *name, const vectorArray matrix[MATRIX_DIM])
{
    unsigned row;
    unsigned col;
    xil_printf("%s =\r\n", name);
    for (row = 0U; row < MATRIX_DIM; ++row) {
        for (col = 0U; col < MATRIX_DIM; ++col) {
            xil_printf("%4d", matrix[row].comp[col]);
        }
        xil_printf("\r\n");
    }
}

static void runSoftwareMultiply(void)
{
    u32 elapsed;
    setInputMatrices();
    ErtsTimer_Start(&timer);
    multiMatrixSoft(aInst, bTInst, pInst);
    elapsed = ErtsTimer_Stop(&timer);

    displayMatrix("A", aInst);
    displayMatrix("B^T", bTInst);
    displayMatrix("P = A * B^T", pInst);
    xil_printf("Software execution time: %u timer ticks\r\n", elapsed);
}

int main(void)
{
    char command[16];

    init_platform();
    if (ErtsTimer_Init(&timer) != XST_SUCCESS) {
        xil_printf("Timer initialization failed.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }
    xil_printf("\r\nERTS Assignment 2 - Exercise 4\r\n");
    xil_printf("Commands: mult, help\r\n");
    for (;;) {
        xil_printf("CMD:> ");
        Console_ReadLine(command, sizeof command);
        if (strcmp(command, "mult") == 0) {
            runSoftwareMultiply();
        } else if (strcmp(command, "help") == 0) {
            xil_printf("mult  calculate P=A*B^T in ARM software\r\n");
        } else {
            xil_printf("Unknown command '%s'.\r\n", command);
        }
    }
}

