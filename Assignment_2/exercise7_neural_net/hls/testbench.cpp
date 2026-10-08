#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "neuralNet.h"

static void referenceNetwork(const char input[INPUT_LEN],
                             const short weightsI[WEIGHTS_I_LEN],
                             const short weightsO[WEIGHTS_O_LEN],
                             int output[OUTPUT_LEN])
{
    int neurons[NEURONS_LEN];
    int i;
    int j;

    for (i = 0; i < NEURONS_LEN; ++i) {
        long long accumulator = 0;
        for (j = 0; j < INPUT_LEN; ++j) {
            accumulator += (long long)input[j] * weightsI[i * INPUT_LEN + j];
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
                (long long)neurons[j] * weightsO[i * NEURONS_LEN + j];
        }
        output[i] = (int)(accumulator >> BITS);
    }
}

int main(void)
{
    char inputs[INPUT_LEN];
    short weightsI[WEIGHTS_I_LEN];
    short weightsO[WEIGHTS_O_LEN];
    int actual[OUTPUT_LEN];
    int expected[OUTPUT_LEN];
    ap_int<SL_LEN> switches = 0x7;
    ap_int<SL_LEN> leds = 0;
    int failures = 0;
    int i;

    for (i = 0; i < INPUT_LEN; ++i) {
        inputs[i] = (char)((i % 31) - 15);
    }
    for (i = 0; i < WEIGHTS_I_LEN; ++i) {
        const double value = ((i % 17) - 8) / 64.0;
        weightsI[i] = (short)std::lround(value * (1 << BITS));
    }
    for (i = 0; i < WEIGHTS_O_LEN; ++i) {
        const double value = ((i % 13) - 6) / 32.0;
        weightsO[i] = (short)std::lround(value * (1 << BITS));
    }

    referenceNetwork(inputs, weightsI, weightsO, expected);
    neuralNet(inputs, weightsI, weightsO, actual, switches, &leds);

    if ((int)leds != (int)switches) {
        std::printf("LED pass-through failed: %d != %d\n", (int)leds,
                    (int)switches);
        ++failures;
    }
    for (i = 0; i < OUTPUT_LEN; ++i) {
        std::printf("output[%d] = %d (expected %d)\n", i, actual[i],
                    expected[i]);
        if (actual[i] != expected[i]) {
            ++failures;
        }
    }

    std::printf("neural network test: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

