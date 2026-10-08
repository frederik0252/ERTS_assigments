#include <string.h>

#include "platform.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xstatus.h"

#include "console.h"
#include "zynq_timer.h"

#define GPIO_CHANNEL 1U
#define LED_MASK 0x0FU
#define TIMER_TICKS_PER_SECOND 325000000U

static XGpio switches;
static XGpio leds;
static ErtsTimer timer;

static int initHardware(void)
{
    if (XGpio_Initialize(&switches, XPAR_SWITCHES_DEVICE_ID) != XST_SUCCESS ||
        XGpio_Initialize(&leds, XPAR_LEDS_DEVICE_ID) != XST_SUCCESS ||
        ErtsTimer_Init(&timer) != XST_SUCCESS) {
        return XST_FAILURE;
    }
    XGpio_SetDataDirection(&switches, GPIO_CHANNEL, LED_MASK);
    XGpio_SetDataDirection(&leds, GPIO_CHANNEL, 0U);
    XGpio_DiscreteWrite(&leds, GPIO_CHANNEL, 0U);
    return XST_SUCCESS;
}

static void showHelp(void)
{
    xil_printf("1     copy SW0-SW3 to LED0-LED3\r\n");
    xil_printf("2     count 0..15 on LEDs (one step/second)\r\n");
    xil_printf("help  show commands\r\n");
}

static void copySwitches(void)
{
    const u32 value = XGpio_DiscreteRead(&switches, GPIO_CHANNEL) & LED_MASK;
    XGpio_DiscreteWrite(&leds, GPIO_CHANNEL, value);
    xil_printf("Switch value: 0x%x\r\n", value);
}

static void runCounter(void)
{
    u32 value;
    for (value = 0U; value <= LED_MASK; ++value) {
        XGpio_DiscreteWrite(&leds, GPIO_CHANNEL, value);
        xil_printf("Counter: %d\r\n", value);
        ErtsTimer_DelayTicks(&timer, TIMER_TICKS_PER_SECOND);
    }
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
        } else if (strcmp(command, "help") == 0) {
            showHelp();
        } else {
            xil_printf("Unknown command '%s'.\r\n", command);
        }
    }
}

