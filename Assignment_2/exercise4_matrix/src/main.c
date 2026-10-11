#include <string.h>

#include "platform.h"
#include "xstatus.h"

#include "console.h"
#include "matrix.h"
#include "zynq_timer.h"

static ErtsTimer timer;

static void runSoftwareMultiply(void)
{
    u32 elapsed;
    setInputMatrices(aInst, bTInst);
    ErtsTimer_Start(&timer);
    multiMatrixSoft(aInst, bTInst, pInst);
    elapsed = ErtsTimer_Stop(&timer);

    xil_printf("A =\r\n");
    displayMatrix(aInst);
    xil_printf("Stored B^T =\r\n");
    displayMatrix(bTInst);
    xil_printf("P =\r\n");
    displayMatrix(pInst);
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
            xil_printf("mult  multiply using the stored B^T rows in ARM software\r\n");
        } else {
            xil_printf("Unknown command '%s'.\r\n", command);
        }
    }
}
