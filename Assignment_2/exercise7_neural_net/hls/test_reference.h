#ifndef ERTS_NEURAL_NET_TEST_REFERENCE_H
#define ERTS_NEURAL_NET_TEST_REFERENCE_H

#include <cstdio>
#include "neuralNet.h"

/* Explicit floor division models an arithmetic right shift, including negatives. */
static inline int referenceScale(long long accumulator)
{
    const long long scale = 1LL << BITS;
    return (int)(accumulator >= 0 ? accumulator / scale
                                 : -((-accumulator + scale - 1) / scale));
}

static inline void referenceNetwork(const char input[INPUT_LEN],
                                    const short weightsI[WEIGHTS_I_LEN],
                                    const short weightsO[WEIGHTS_O_LEN],
                                    int output[OUTPUT_LEN])
{
    int neurons[NEURONS_LEN];
    const short *weight = weightsI;
    for (int neuron = 0; neuron < NEURONS_LEN; ++neuron) {
        long long sum = 0;
        for (int sample = 0; sample < INPUT_LEN; ++sample) {
            const unsigned byte = (unsigned char)input[sample];
            const int signedSample = byte < 128U ? (int)byte : (int)byte - 256;
            sum += (long long)signedSample * *weight++;
        }
        const int value = referenceScale(sum);
        neurons[neuron] = value > 0 ? value : 0;
    }

    weight = weightsO;
    for (int result = 0; result < OUTPUT_LEN; ++result) {
        long long sum = 0;
        for (int neuron = 0; neuron < NEURONS_LEN; ++neuron) {
            sum += (long long)neurons[neuron] * *weight++;
        }
        output[result] = referenceScale(sum);
    }
}

static inline int checkNetworkCase(const char *name, char input[INPUT_LEN],
                                   short weightsI[WEIGHTS_I_LEN],
                                   short weightsO[WEIGHTS_O_LEN],
                                   unsigned switchPattern)
{
    int actual[OUTPUT_LEN];
    int expected[OUTPUT_LEN];
    ap_int<SL_LEN> switches = switchPattern;
    ap_int<SL_LEN> leds = 0;
    int failures = 0;
    for (int i = 0; i < OUTPUT_LEN; ++i) {
        actual[i] = 0x13579BDF;
    }

    referenceNetwork(input, weightsI, weightsO, expected);
    neuralNet(input, weightsI, weightsO, actual, switches, &leds);
    std::printf("CASE: %s (%d/%d/%d)\n", name, INPUT_LEN, NEURONS_LEN,
                OUTPUT_LEN);
    if ((int)leds != (int)switches) {
        std::printf("LED pass-through failed: %d != %d\n", (int)leds,
                    (int)switches);
        ++failures;
    }
    for (int i = 0; i < OUTPUT_LEN; ++i) {
        std::printf("output[%d] = %d (expected %d)\n", i, actual[i], expected[i]);
        if (actual[i] != expected[i]) {
            ++failures;
        }
    }
    std::printf("case result: %s\n", failures == 0 ? "PASS" : "FAIL");
    return failures;
}

int testCourseDerivedNetwork(void);

#endif
