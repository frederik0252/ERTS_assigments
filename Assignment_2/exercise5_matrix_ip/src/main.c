#include <string.h>

#include "matrix_ip.h"
#include "platform.h"
#include "xil_io.h"
#include "xparameters.h"
#include "xstatus.h"

#include "console.h"
#include "matrix.h"
#include "zynq_timer.h"

#ifndef MATRIX_IP_S00_AXI_SLV_REG0_OFFSET
#define MATRIX_IP_S00_AXI_SLV_REG0_OFFSET 0U
#define MATRIX_IP_S00_AXI_SLV_REG1_OFFSET 4U
#define MATRIX_IP_S00_AXI_SLV_REG2_OFFSET 8U
#endif

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

static void multiMatrixHard(const vectorArray a[MATRIX_DIM],
                            const vectorArray bT[MATRIX_DIM],
                            vectorArray product[MATRIX_DIM])
{
    unsigned row;
    unsigned col;
    for (row = 0U; row < MATRIX_DIM; ++row) {
        for (col = 0U; col < MATRIX_DIM; ++col) {
            Xil_Out32(XPAR_MATRIX_IP_0_S00_AXI_BASEADDR +
                          MATRIX_IP_S00_AXI_SLV_REG0_OFFSET,
                      a[row].vect);
            Xil_Out32(XPAR_MATRIX_IP_0_S00_AXI_BASEADDR +
                          MATRIX_IP_S00_AXI_SLV_REG1_OFFSET,
                      bT[col].vect);
            product[row].comp[col] = (uint8_t)Xil_In32(
                XPAR_MATRIX_IP_0_S00_AXI_BASEADDR +
                MATRIX_IP_S00_AXI_SLV_REG2_OFFSET);
        }
    }
}

static void runBenchmark(void)
{
    vectorArray software[MATRIX_DIM];
    vectorArray hardware[MATRIX_DIM];
    u32 softwareTicks;
    u32 hardwareTicks;

    setInputMatrices();
    ErtsTimer_Start(&timer);
    multiMatrixSoft(aInst, bTInst, software);
    softwareTicks = ErtsTimer_Stop(&timer);

    ErtsTimer_Start(&timer);
    multiMatrixHard(aInst, bTInst, hardware);
    hardwareTicks = ErtsTimer_Stop(&timer);

    displayMatrix("A", aInst);
    displayMatrix("B^T", bTInst);
    displayMatrix("P (software)", software);
    displayMatrix("P (hardware)", hardware);
    xil_printf("Software: %u ticks\r\n", softwareTicks);
    xil_printf("Hardware: %u ticks\r\n", hardwareTicks);
    xil_printf("Verification: %s\r\n",
               matrixEquals(software, hardware) ? "PASS" : "FAIL");
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
    xil_printf("\r\nERTS Assignment 2 - Exercise 5\r\n");
    xil_printf("Commands: mult, help\r\n");
    for (;;) {
        xil_printf("CMD:> ");
        Console_ReadLine(command, sizeof command);
        if (strcmp(command, "mult") == 0) {
            runBenchmark();
        } else if (strcmp(command, "help") == 0) {
            xil_printf("mult  compare software and AXI-IP matrix multiply\r\n");
        } else {
            xil_printf("Unknown command '%s'.\r\n", command);
        }
    }
}

