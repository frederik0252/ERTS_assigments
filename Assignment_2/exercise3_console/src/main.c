#include <string.h>

#include "platform.h"
#include "xgpio.h"
#include "xil_io.h"
#include "xparameters.h"
#include "xstatus.h"

#include "console.h"
#include "matrix.h"
#include "zynq_peripherals.h"
#include "zynq_timer.h"

#ifndef ERTS_SWITCHES_CONFIG_KEY
#error "The platform must expose switches as an AXI GPIO instance"
#endif
#ifndef ERTS_LED_BASEADDR
#error "The platform must expose the course custom led_ip instance"
#endif

#define GPIO_CHANNEL 1U
#define LED_MASK 0x0FU
#define TIMER_TICKS_PER_SECOND 325000000U

static XGpio switches;
static ErtsTimer timer;

static void setLeds(u32 value)
{
    /* Register 0 drives LED[3:0] in the supplied custom LED IP. */
    Xil_Out32(ERTS_LED_BASEADDR, value & LED_MASK);
}

static int initHardware(void)
{
    if (XGpio_Initialize(&switches, ERTS_SWITCHES_CONFIG_KEY) != XST_SUCCESS ||
        ErtsTimer_Init(&timer) != XST_SUCCESS) {
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&switches, GPIO_CHANNEL, LED_MASK);
    setLeds(0U);
    return XST_SUCCESS;
}

static void showHelp(void)
{
    xil_printf("1     copy SW0-SW3 to LED0-LED3\r\n");
    xil_printf("2     count 0..15 on LEDs (one step/second)\r\n");
    xil_printf("mult  multiply the Exercise 4 matrices in software\r\n");
    xil_printf("help  show commands\r\n");
}

static void copySwitches(void)
{
    const u32 value = XGpio_DiscreteRead(&switches, GPIO_CHANNEL) & LED_MASK;
    setLeds(value);
    xil_printf("Switch value: 0x%x\r\n", value);
}

static void runCounter(void)
{
    u32 value;

    /* Reload continuously so UART printing is not added to every second. */
    XScuTimer_Stop(&timer.instance);
    XScuTimer_LoadTimer(&timer.instance, TIMER_TICKS_PER_SECOND);
    XScuTimer_ClearInterruptStatus(&timer.instance);
    XScuTimer_EnableAutoReload(&timer.instance);
    XScuTimer_Start(&timer.instance);
    for (value = 0U; value <= LED_MASK; ++value) {
        setLeds(value);
        xil_printf("Counter: %d\r\n", value);
        if (value != LED_MASK) {
            while (!XScuTimer_IsExpired(&timer.instance)) {
                /* Timer polling is intentional in this bare-metal console. */
            }
            XScuTimer_ClearInterruptStatus(&timer.instance);
        }
    }
    XScuTimer_Stop(&timer.instance);
    XScuTimer_DisableAutoReload(&timer.instance);
    XScuTimer_ClearInterruptStatus(&timer.instance);
}

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
    if (initHardware() != XST_SUCCESS) {
        xil_printf("Hardware initialization failed.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }

    xil_printf("\r\nERTS Assignment 2 - Exercise 3\r\n");
    showHelp();
    for (;;) {
        xil_printf("CMD:> ");
        Console_ReadLine(command, sizeof command);
        if (strcmp(command, "1") == 0) {
            copySwitches();
        } else if (strcmp(command, "2") == 0) {
            runCounter();
        } else if (strcmp(command, "mult") == 0) {
            runSoftwareMultiply();
        } else if (strcmp(command, "help") == 0) {
            showHelp();
        } else {
            xil_printf("Unknown command '%s'.\r\n", command);
        }
    }
}
