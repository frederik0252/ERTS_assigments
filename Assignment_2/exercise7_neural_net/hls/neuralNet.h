#ifndef ERTS_NEURAL_NET_H
#define ERTS_NEURAL_NET_H

#include "ap_int.h"
#include "neuralNet_config.h"

void neuralNet(char input[INPUT_LEN], short weightsI[WEIGHTS_I_LEN],
               short weightsO[WEIGHTS_O_LEN], int output[OUTPUT_LEN],
               ap_int<SL_LEN> inSwitch, ap_int<SL_LEN> *outLeds);

#endif
