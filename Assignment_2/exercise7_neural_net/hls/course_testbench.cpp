/*
 * Adapted from Assignment_2/Ex7ANN/testbench.cpp. The untouched source is in
 * provided_course/. Keep its input/weight generation; replace its overlapping
 * weight-row strides and floating-point oracle with the specified fixed-point
 * model in test_reference.h. This is not a claim that the original test passes.
 */
#include "test_reference.h"

int testCourseDerivedNetwork(void)
{
    char inputs[INPUT_LEN];
    short weightsIS[WEIGHTS_I_LEN];
    float weightsI[WEIGHTS_I_LEN];
    short weightsOS[WEIGHTS_O_LEN];
    float weightsO[WEIGHTS_O_LEN];
    const int scale = 1 << BITS;

    for (int i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = i % 255;
    }
    for (int i = 0; i < WEIGHTS_I_LEN; ++i) {
        weightsI[i] = 0.0001 * i;
        weightsIS[i] = weightsI[i] * scale + 0.5;
    }
    for (int i = 0; i < WEIGHTS_O_LEN; ++i) {
        weightsO[i] = 0.001 * i;
        weightsOS[i] = weightsO[i] * scale + 0.5;
    }

    return checkNetworkCase("adapted course vector: corrected fixed-point oracle",
                            inputs, weightsIS, weightsOS, 0x7U);
}
