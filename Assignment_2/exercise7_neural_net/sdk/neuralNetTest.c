#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "xneuralnet.h"
#include "xparameters.h"
#include "xstatus.h"

#include "zynq_timer.h"

#define BITS 15
#define INPUT_LEN 196
#define NEURONS_LEN 32
#define WEIGHTS_I_LEN (INPUT_LEN * NEURONS_LEN)
#define OUTPUT_LEN 10
#define WEIGHTS_O_LEN (NEURONS_LEN * OUTPUT_LEN)

static XNeuralnet accelerator;
static ErtsTimer timer;
static signed char inputs[INPUT_LEN];
static short weightsI[WEIGHTS_I_LEN];
static short weightsO[WEIGHTS_O_LEN];
static int softwareOutput[OUTPUT_LEN];
static int hardwareOutput[OUTPUT_LEN];

static void neuralNetSoftware(const signed char input[INPUT_LEN],
                              const short inputWeights[WEIGHTS_I_LEN],
                              const short outputWeights[WEIGHTS_O_LEN],
                              int output[OUTPUT_LEN])
{
    int neurons[NEURONS_LEN];
    int i;
    int j;

    for (i = 0; i < NEURONS_LEN; ++i) {
        long long accumulator = 0;
        for (j = 0; j < INPUT_LEN; ++j) {
            accumulator +=
                (long long)input[j] * inputWeights[i * INPUT_LEN + j];
        }
        neurons[i] = (int)(accumulator >> BITS);
        if (neurons[i] < 0) {
            neurons[i] = 0;
        }
    }
    for (i = 0; i < OUTPUT_LEN; ++i) {
        long long accumulator = 0;
        for (j = 0; j < NEURONS_LEN; ++j) {
            accumulator +=
                (long long)neurons[j] * outputWeights[i * NEURONS_LEN + j];
        }
        output[i] = (int)(accumulator >> BITS);
    }
}

static void createTestData(void)
{
    int i;
    for (i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = (signed char)((i % 31) - 15);
    }
    for (i = 0; i < WEIGHTS_I_LEN; ++i) {
        weightsI[i] = (short)(((i % 17) - 8) * 512);
    }
    for (i = 0; i < WEIGHTS_O_LEN; ++i) {
        weightsO[i] = (short)(((i % 13) - 6) * 1024);
    }
}

static u32 loadAccelerator(void)
{
    u32 ticks;
    ErtsTimer_Start(&timer);
    XNeuralnet_Write_input_r_Words(&accelerator, 0, (int *)inputs,
                                   INPUT_LEN / 4);
    XNeuralnet_Write_weightsI_Words(&accelerator, 0, (int *)weightsI,
                                    WEIGHTS_I_LEN / 2);
    XNeuralnet_Write_weightsO_Words(&accelerator, 0, (int *)weightsO,
                                    WEIGHTS_O_LEN / 2);
    ticks = ErtsTimer_Stop(&timer);
    return ticks;
}

static u32 runAccelerator(void)
{
    u32 ticks;
    while (!XNeuralnet_IsReady(&accelerator)) {
    }
    ErtsTimer_Start(&timer);
    XNeuralnet_Start(&accelerator);
    while (!XNeuralnet_IsDone(&accelerator)) {
    }
    ticks = ErtsTimer_Stop(&timer);
    XNeuralnet_Read_output_r_Words(&accelerator, 0, hardwareOutput, OUTPUT_LEN);
    return ticks;
}

int main(void)
{
    u32 softwareTicks;
    u32 loadTicks;
    u32 hardwareTicks;
    int failures = 0;
    int i;

    init_platform();
    xil_printf("\r\nERTS Assignment 2 - Exercise 7\r\n");
    if (ErtsTimer_Init(&timer) != XST_SUCCESS ||
        XNeuralnet_Initialize(&accelerator, XPAR_NEURALNET_0_DEVICE_ID) !=
            XST_SUCCESS) {
        xil_printf("Hardware initialization failed.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }

    createTestData();
    ErtsTimer_Start(&timer);
    neuralNetSoftware(inputs, weightsI, weightsO, softwareOutput);
    softwareTicks = ErtsTimer_Stop(&timer);
    loadTicks = loadAccelerator();
    hardwareTicks = runAccelerator();

    for (i = 0; i < OUTPUT_LEN; ++i) {
        xil_printf("output[%d] SW=%d HW=%d %s\r\n", i, softwareOutput[i],
                   hardwareOutput[i],
                   softwareOutput[i] == hardwareOutput[i] ? "PASS" : "FAIL");
        if (softwareOutput[i] != hardwareOutput[i]) {
            ++failures;
        }
    }
    xil_printf("CPU compute:     %u ticks\r\n", softwareTicks);
    xil_printf("HW data loading: %u ticks\r\n", loadTicks);
    xil_printf("HW compute:      %u ticks\r\n", hardwareTicks);
    xil_printf("Overall verification: %s\r\n", failures == 0 ? "PASS" : "FAIL");

    cleanup_platform();
    return failures == 0 ? XST_SUCCESS : XST_FAILURE;
}

