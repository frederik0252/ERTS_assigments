#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "xneuralnet.h"
#include "xparameters.h"
#include "xstatus.h"

#include "neuralNet_config.h"
#include "zynq_timer.h"

#define ACCELERATOR_POLL_LIMIT 100000000U
#define BENCHMARK_TRIALS 10U
#define EXPECTED_ARRAY_BASE_ADDRESS 0x40000000U
#define EXPECTED_CONTROL_BASE_ADDRESS 0x43C00000U
#define WORDS_FOR_BYTES(byte_count)                                        \
    (((byte_count) + sizeof(word_type) - 1U) / sizeof(word_type))

enum {
    INPUT_WORDS = WORDS_FOR_BYTES(INPUT_LEN * sizeof(signed char)),
    WEIGHTS_I_WORDS = WORDS_FOR_BYTES(WEIGHTS_I_LEN * sizeof(short)),
    WEIGHTS_O_WORDS = WORDS_FOR_BYTES(WEIGHTS_O_LEN * sizeof(short)),
    OUTPUT_WORDS = WORDS_FOR_BYTES(OUTPUT_LEN * sizeof(int))
};

#ifndef XNEURALNET_BUS_R_DEPTH_INPUT_R
#error "The generated neuralNet driver does not expose the expected array map"
#endif
#if XNEURALNET_BUS_R_DEPTH_INPUT_R != INPUT_LEN ||                         \
    XNEURALNET_BUS_R_DEPTH_WEIGHTSI != WEIGHTS_I_LEN ||                   \
    XNEURALNET_BUS_R_DEPTH_WEIGHTSO != WEIGHTS_O_LEN ||                   \
    XNEURALNET_BUS_R_DEPTH_OUTPUT_R != OUTPUT_LEN
#error "neuralNet_config.h dimensions do not match the generated HLS driver"
#endif
#if XNEURALNET_BUS_R_WIDTH_INPUT_R != 8 ||                                \
    XNEURALNET_BUS_R_WIDTH_WEIGHTSI != 16 ||                              \
    XNEURALNET_BUS_R_WIDTH_WEIGHTSO != 16 ||                              \
    XNEURALNET_BUS_R_WIDTH_OUTPUT_R != 32
#error "The generated neuralNet driver has unexpected array element widths"
#endif

#ifdef SDT
#ifndef XPAR_NEURALNET_0_BASEADDR_1
#error "The Ex7 SDT platform must expose both neuralNet AXI base addresses"
#endif
#if XPAR_NEURALNET_0_BASEADDR != EXPECTED_ARRAY_BASE_ADDRESS
#error "Unexpected neuralNet array-bus address in the generated SDT platform"
#endif
#if XPAR_NEURALNET_0_BASEADDR_1 != EXPECTED_CONTROL_BASE_ADDRESS
#error "Unexpected neuralNet control-bus address in the generated SDT platform"
#endif
#endif

typedef struct {
    u32 load;
    u32 run;
    u32 full;
} HardwareTiming;

typedef struct {
    u32 minimum;
    u32 maximum;
    u64 total;
} TickStatistics;

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
static word_type packedHardwareOutput[OUTPUT_WORDS];
static volatile int benchmarkSink;

static int validateDataLayout(void)
{
    return CHAR_BIT == 8 && sizeof(short) == 2U && sizeof(int) == 4U &&
                   sizeof(word_type) == 4U
               ? XST_SUCCESS
               : XST_FAILURE;
}

static int initializeAccelerator(void)
{
#ifdef SDT
    /*
     * Vitis 2026.1 emits this dual-interface HLS IP's SDT reg entries in
     * ascending address order.  The generated names are therefore opposite
     * the driver's semantic BUS (control) and BUS_r (array memory) fields.
     * Refuse a changed address map at compile time above, then initialize the
     * named fields explicitly here.
     */
    XNeuralnet_Config config = {
        .Name = XPAR_NEURALNET_0_COMPATIBLE,
        .Bus_BaseAddress = XPAR_NEURALNET_0_BASEADDR_1,
        .Bus_r_BaseAddress = XPAR_NEURALNET_0_BASEADDR,
    };

    if (config.Bus_BaseAddress != EXPECTED_CONTROL_BASE_ADDRESS ||
        config.Bus_r_BaseAddress != EXPECTED_ARRAY_BASE_ADDRESS) {
        return XST_FAILURE;
    }
    return XNeuralnet_CfgInitialize(&accelerator, &config);
#else
    return XNeuralnet_Initialize(&accelerator,
                                 XPAR_NEURALNET_0_DEVICE_ID);
#endif
}

/* Kept independent of the HLS function so hardware is checked against a
 * separately implemented fixed-point reference. */
static __attribute__((noinline)) void neuralNetSoftware(
    const signed char input[INPUT_LEN],
    const short inputWeights[WEIGHTS_I_LEN],
    const short outputWeights[WEIGHTS_O_LEN], int output[OUTPUT_LEN])
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
    /* Zero the complete word arrays so any final partial word is defined. */
    memset(packedInputs, 0, sizeof(packedInputs));
    memset(packedWeightsI, 0, sizeof(packedWeightsI));
    memset(packedWeightsO, 0, sizeof(packedWeightsO));
    memcpy(packedInputs, inputs, sizeof(inputs));
    memcpy(packedWeightsI, weightsI, sizeof(weightsI));
    memcpy(packedWeightsO, weightsO, sizeof(weightsO));
}

static int writeAcceleratorInputs(void)
{
    const u32 inputWords = XNeuralnet_Write_input_r_Words(
        &accelerator, 0, packedInputs, INPUT_WORDS);
    const u32 inputWeightWords = XNeuralnet_Write_weightsI_Words(
        &accelerator, 0, packedWeightsI, WEIGHTS_I_WORDS);
    const u32 outputWeightWords = XNeuralnet_Write_weightsO_Words(
        &accelerator, 0, packedWeightsO, WEIGHTS_O_WORDS);

    return inputWords == INPUT_WORDS &&
                   inputWeightWords == WEIGHTS_I_WORDS &&
                   outputWeightWords == WEIGHTS_O_WORDS
               ? XST_SUCCESS
               : XST_FAILURE;
}

static int waitUntilReady(void)
{
    u32 polls = ACCELERATOR_POLL_LIMIT;

    while (!XNeuralnet_IsReady(&accelerator)) {
        if (--polls == 0U) {
            return XST_FAILURE;
        }
    }
    return XST_SUCCESS;
}

static int waitUntilDone(void)
{
    u32 polls = ACCELERATOR_POLL_LIMIT;

    while (!XNeuralnet_IsDone(&accelerator)) {
        if (--polls == 0U) {
            return XST_FAILURE;
        }
    }
    return XST_SUCCESS;
}

static u32 timerCounter(void)
{
    return XScuTimer_GetCounterValue(&timer.instance);
}

static int runHardwareTransaction(HardwareTiming *timing)
{
    u32 loadStart;
    u32 runStart;

    /* FULL deliberately includes packing, writes, readiness, execution,
     * readback and unpacking.  Phase snapshots use the same running timer so
     * they do not perturb this end-to-end measurement. */
    ErtsTimer_Start(&timer);
    packAcceleratorData();

    loadStart = timerCounter();
    if (writeAcceleratorInputs() != XST_SUCCESS) {
        (void)ErtsTimer_Stop(&timer);
        return XST_FAILURE;
    }
    timing->load = loadStart - timerCounter();

    if (waitUntilReady() != XST_SUCCESS) {
        (void)ErtsTimer_Stop(&timer);
        return XST_FAILURE;
    }
    runStart = timerCounter();
    XNeuralnet_Start(&accelerator);
    if (waitUntilDone() != XST_SUCCESS) {
        (void)ErtsTimer_Stop(&timer);
        return XST_FAILURE;
    }
    timing->run = runStart - timerCounter();

    if (XNeuralnet_Read_output_r_Words(
            &accelerator, 0, packedHardwareOutput, OUTPUT_WORDS) !=
        OUTPUT_WORDS) {
        (void)ErtsTimer_Stop(&timer);
        return XST_FAILURE;
    }
    memcpy(hardwareOutput, packedHardwareOutput, sizeof(hardwareOutput));
    timing->full = ErtsTimer_Stop(&timer);
    return XST_SUCCESS;
}

static u32 runSoftwareReference(void)
{
    int i;
    int checksum = 0;

    ErtsTimer_Start(&timer);
    neuralNetSoftware(inputs, weightsI, weightsO, softwareOutput);
    {
        const u32 ticks = ErtsTimer_Stop(&timer);
        for (i = 0; i < OUTPUT_LEN; ++i) {
            checksum ^= softwareOutput[i];
        }
        benchmarkSink = checksum;
        return ticks;
    }
}

static int compareOutputs(int printValues)
{
    int failures = 0;
    int i;

    for (i = 0; i < OUTPUT_LEN; ++i) {
        const int match = softwareOutput[i] == hardwareOutput[i];
        if (printValues) {
            xil_printf("output[%d] SW=%d HW=%d %s\r\n", i,
                       softwareOutput[i], hardwareOutput[i],
                       match ? "PASS" : "FAIL");
        }
        if (!match) {
            ++failures;
        }
    }
    return failures;
}

static void initializeStatistics(TickStatistics *statistics)
{
    statistics->minimum = 0xFFFFFFFFU;
    statistics->maximum = 0U;
    statistics->total = 0U;
}

static void addStatistic(TickStatistics *statistics, u32 ticks)
{
    if (ticks < statistics->minimum) {
        statistics->minimum = ticks;
    }
    if (ticks > statistics->maximum) {
        statistics->maximum = ticks;
    }
    statistics->total += ticks;
}

static void printStatistics(const char *label,
                            const TickStatistics *statistics)
{
    const u32 mean = (u32)(statistics->total / BENCHMARK_TRIALS);
    xil_printf("%s (%u trials): min=%u mean=%u max=%u ticks\r\n", label,
               BENCHMARK_TRIALS, statistics->minimum, mean,
               statistics->maximum);
}

int main(void)
{
    HardwareTiming firstHardwareTiming;
    TickStatistics softwareStatistics;
    TickStatistics loadStatistics;
    TickStatistics runStatistics;
    TickStatistics fullStatistics;
    u32 firstSoftwareTicks;
    unsigned int trial;
    int failures = 0;

    init_platform();
    xil_printf("\r\nERTS Assignment 2 - Exercise 7\r\n");
    xil_printf("Dimensions: input=%d hidden=%d output=%d Q bits=%d\r\n",
               INPUT_LEN, NEURONS_LEN, OUTPUT_LEN, BITS);
    if (validateDataLayout() != XST_SUCCESS ||
        ErtsTimer_Init(&timer) != XST_SUCCESS ||
        initializeAccelerator() != XST_SUCCESS) {
        xil_printf("Hardware configuration or initialization failed.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }

    createTestData();
    firstSoftwareTicks = runSoftwareReference();
    if (runHardwareTransaction(&firstHardwareTiming) != XST_SUCCESS) {
        xil_printf("Accelerator transfer or execution timed out.\r\n");
        cleanup_platform();
        return XST_FAILURE;
    }
    failures += compareOutputs(1);

    xil_printf("First CPU compute:         %u ticks\r\n", firstSoftwareTicks);
    xil_printf("First HW data loading:     %u ticks\r\n",
               firstHardwareTiming.load);
    xil_printf("First HW compute:          %u ticks\r\n",
               firstHardwareTiming.run);
    xil_printf("First HW FULL transaction: %u ticks\r\n",
               firstHardwareTiming.full);

    initializeStatistics(&softwareStatistics);
    initializeStatistics(&loadStatistics);
    initializeStatistics(&runStatistics);
    initializeStatistics(&fullStatistics);
    for (trial = 0U; trial < BENCHMARK_TRIALS; ++trial) {
        HardwareTiming hardwareTiming;
        addStatistic(&softwareStatistics, runSoftwareReference());
        if (runHardwareTransaction(&hardwareTiming) != XST_SUCCESS) {
            xil_printf("Benchmark trial %u timed out.\r\n", trial + 1U);
            cleanup_platform();
            return XST_FAILURE;
        }
        addStatistic(&loadStatistics, hardwareTiming.load);
        addStatistic(&runStatistics, hardwareTiming.run);
        addStatistic(&fullStatistics, hardwareTiming.full);
        failures += compareOutputs(0);
    }

    printStatistics("CPU reference", &softwareStatistics);
    printStatistics("HW data loading", &loadStatistics);
    printStatistics("HW compute", &runStatistics);
    printStatistics("HW FULL transaction", &fullStatistics);
    xil_printf("Overall verification: %s\r\n",
               failures == 0 ? "PASS" : "FAIL");

    cleanup_platform();
    return failures == 0 ? XST_SUCCESS : XST_FAILURE;
}
