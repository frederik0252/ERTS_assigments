#include <string.h>

#include "platform.h"
#include "xil_io.h"
#include "xparameters.h"
#include "xstatus.h"

#include "console.h"
#include "matrix.h"
#include "zynq_peripherals.h"
#include "zynq_timer.h"

#ifndef ERTS_MATRIX_BASEADDR
#error "The platform must expose the course matrix_ip instance"
#endif

/* Register offsets from the supplied matrix_ip_1.0 RTL. No BSP driver needed. */
#define MATRIX_IP_S00_AXI_SLV_REG0_OFFSET 0U
#define MATRIX_IP_S00_AXI_SLV_REG1_OFFSET 4U
#define MATRIX_IP_S00_AXI_SLV_REG2_OFFSET 8U

static ErtsTimer timer;

void multiMatrixHard(VectorArray A, VectorArray B, VectorArray P)
{
    unsigned row;
    unsigned col;
    for (row = 0U; row < MSIZE; ++row) {
        for (col = 0U; col < MSIZE; ++col) {
            Xil_Out32(ERTS_MATRIX_BASEADDR +
                          MATRIX_IP_S00_AXI_SLV_REG0_OFFSET,
                      A[row].vect);
            Xil_Out32(ERTS_MATRIX_BASEADDR +
                          MATRIX_IP_S00_AXI_SLV_REG1_OFFSET,
                      B[col].vect);
            P[row].comp[col] = (unsigned char)Xil_In32(
                ERTS_MATRIX_BASEADDR +
                MATRIX_IP_S00_AXI_SLV_REG2_OFFSET);
        }
    }
}

static void runBenchmark(void)
{
    VectorArray software;
    VectorArray hardware;
    u32 softwareTicks;
    u32 hardwareTicks;

    setInputMatrices(aInst, bTInst);
    ErtsTimer_Start(&timer);
    multiMatrixSoft(aInst, bTInst, software);
    softwareTicks = ErtsTimer_Stop(&timer);

    ErtsTimer_Start(&timer);
    multiMatrixHard(aInst, bTInst, hardware);
    hardwareTicks = ErtsTimer_Stop(&timer);

    xil_printf("A =\r\n");
    displayMatrix(aInst);
    xil_printf("Stored B^T =\r\n");
    displayMatrix(bTInst);
    xil_printf("P (software) =\r\n");
    displayMatrix(software);
    xil_printf("P (hardware) =\r\n");
    displayMatrix(hardware);
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
