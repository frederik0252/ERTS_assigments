#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define INPUT_WORDS (INPUT_LEN / 4)
#define WEIGHTS_I_WORDS (WEIGHTS_I_LEN / 2)
#define WEIGHTS_O_WORDS (WEIGHTS_O_LEN / 2)
#define ACCELERATOR_POLL_LIMIT 100000000U

static XNeuralnet accelerator;
static ErtsTimer timer;
static signed char inputs[INPUT_LEN];
static short weightsI[WEIGHTS_I_LEN];
static short weightsO[WEIGHTS_O_LEN];
static int softwareOutput[OUTPUT_LEN];
static int hardwareOutput[OUTPUT_LEN];
static word_type packedInputs[INPUT_WORDS];
static word_type packedWeightsI[WEIGHTS_I_WORDS];
static word_type packedWeightsO[WEIGHTS_O_WORDS];
static word_type packedHardwareOutput[OUTPUT_LEN];

static int initializeAccelerator(void)
{
#ifdef SDT
    /*
     * Vitis 2026.1 emits this dual-interface HLS IP's SDT reg entries in
     * ascending address order.  That reverses the named BUS (control) and
     * BUS_r (array memory) fields in the generated configuration table, so
     * initialize the named driver fields explicitly.
     */
    XNeuralnet_Config config = {
        .Name = XPAR_NEURALNET_0_COMPATIBLE,
        .Bus_BaseAddress = XPAR_NEURALNET_0_BASEADDR_1,
        .Bus_r_BaseAddress = XPAR_NEURALNET_0_BASEADDR,
    };

    return XNeuralnet_CfgInitialize(&accelerator, &config);
#else
    return XNeuralnet_Initialize(&accelerator,
                                 XPAR_NEURALNET_0_DEVICE_ID);
#endif
}

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

static void packAcceleratorData(void)
{
    memcpy(packedInputs, inputs, sizeof(inputs));
    memcpy(packedWeightsI, weightsI, sizeof(weightsI));
    memcpy(packedWeightsO, weightsO, sizeof(weightsO));
}

static int loadAccelerator(u32 *ticks)
{
    u32 inputWords;
    u32 inputWeightWords;
    u32 outputWeightWords;

    ErtsTimer_Start(&timer);
    inputWords = XNeuralnet_Write_input_r_Words(
        &accelerator, 0, packedInputs, INPUT_WORDS);
    inputWeightWords = XNeuralnet_Write_weightsI_Words(
        &accelerator, 0, packedWeightsI, WEIGHTS_I_WORDS);
    outputWeightWords = XNeuralnet_Write_weightsO_Words(
        &accelerator, 0, packedWeightsO, WEIGHTS_O_WORDS);
    *ticks = ErtsTimer_Stop(&timer);

    return inputWords == INPUT_WORDS &&
                   inputWeightWords == WEIGHTS_I_WORDS &&
                   outputWeightWords == WEIGHTS_O_WORDS
               ? XST_SUCCESS
               : XST_FAILURE;
}

static int runAccelerator(u32 *ticks)
{
    u32 polls = ACCELERATOR_POLL_LIMIT;

    while (!XNeuralnet_IsReady(&accelerator)) {
        if (--polls == 0U) {
            return XST_FAILURE;
        }
    }
    ErtsTimer_Start(&timer);
    XNeuralnet_Start(&accelerator);
    polls = ACCELERATOR_POLL_LIMIT;
    while (!XNeuralnet_IsDone(&accelerator)) {
        if (--polls == 0U) {
            (void)ErtsTimer_Stop(&timer);
            return XST_FAILURE;
        }
    }
    *ticks = ErtsTimer_Stop(&timer);
    if (XNeuralnet_Read_output_r_Words(
            &accelerator, 0, packedHardwareOutput, OUTPUT_LEN) != OUTPUT_LEN) {
        return XST_FAILURE;
    }
    memcpy(hardwareOutput, packedHardwareOutput, sizeof(hardwareOutput));
    return XST_SUCCESS;
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
        initializeAccelerator() != XST_SUCCESS) {
        xil_printf("Hardware initialization failed.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }

    createTestData();
    packAcceleratorData();
    ErtsTimer_Start(&timer);
    neuralNetSoftware(inputs, weightsI, weightsO, softwareOutput);
    softwareTicks = ErtsTimer_Stop(&timer);
    if (loadAccelerator(&loadTicks) != XST_SUCCESS ||
        runAccelerator(&hardwareTicks) != XST_SUCCESS) {
        xil_printf("Accelerator transfer or execution timed out.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }

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
