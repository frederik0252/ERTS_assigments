#include <cmath>
#include <cstdlib>

#include "test_reference.h"

int main(void)
{
    char inputs[INPUT_LEN];
    short weightsI[WEIGHTS_I_LEN];
    short weightsO[WEIGHTS_O_LEN];
    int failures = testCourseDerivedNetwork();

    /* Retain the original custom vector used for the recorded board benchmark. */
    for (int i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = (char)((i % 31) - 15);
    }
    for (int i = 0; i < WEIGHTS_I_LEN; ++i) {
        const double value = ((i % 17) - 8) / 64.0;
        weightsI[i] = (short)std::lround(value * (1 << BITS));
    }
    for (int i = 0; i < WEIGHTS_O_LEN; ++i) {
        const double value = ((i % 13) - 6) / 32.0;
        weightsO[i] = (short)std::lround(value * (1 << BITS));
    }
    failures += checkNetworkCase("custom signed vector", inputs, weightsI,
                                  weightsO, 0xAU);

    /* A second call with zero input detects stale results or hidden state. */
    for (int i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = 0;
    }
    failures += checkNetworkCase("zero input after nonzero inference", inputs,
                                  weightsI, weightsO, 0x0U);

    /* Every hidden pre-activation is negative; ReLU must suppress all outputs. */
    for (int i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = (char)-128;
    }
    for (int i = 0; i < WEIGHTS_I_LEN; ++i) {
        weightsI[i] = 32767;
    }
    failures += checkNetworkCase("negative hidden values and ReLU", inputs,
                                  weightsI, weightsO, 0x5U);

    /* Signed extremes exercise wide accumulation and negative output scaling. */
    for (int i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = (char)(i % 2 == 0 ? -128 : 127);
    }
    for (int neuron = 0; neuron < NEURONS_LEN; ++neuron) {
        for (int sample = 0; sample < INPUT_LEN; ++sample) {
            weightsI[neuron * INPUT_LEN + sample] =
                (short)((neuron + sample) % 2 == 0 ? -32768 : 32767);
        }
    }
    for (int output = 0; output < OUTPUT_LEN; ++output) {
        for (int neuron = 0; neuron < NEURONS_LEN; ++neuron) {
            weightsO[output * NEURONS_LEN + neuron] =
                (short)(output % 2 == 0 ? -32768 : 32767);
        }
    }
    failures += checkNetworkCase("signed extrema and wide accumulators", inputs,
                                  weightsI, weightsO, 0xFU);

    std::printf("neural network test: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
