#ifndef ERTS_NEURAL_NET_H
#define ERTS_NEURAL_NET_H

#include "ap_int.h"

#define SL_LEN 4
#define BITS 15
#define INPUT_LEN 196
#define NEURONS_LEN 32
#define WEIGHTS_I_LEN (INPUT_LEN * NEURONS_LEN)
#define OUTPUT_LEN 10
#define WEIGHTS_O_LEN (NEURONS_LEN * OUTPUT_LEN)

void neuralNet(char input[INPUT_LEN], short weightsI[WEIGHTS_I_LEN],
               short weightsO[WEIGHTS_O_LEN], int output[OUTPUT_LEN],
               ap_int<SL_LEN> inSwitch, ap_int<SL_LEN> *outLeds);

#endif

