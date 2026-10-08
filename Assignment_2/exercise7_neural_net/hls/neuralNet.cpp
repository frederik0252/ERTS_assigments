#include "neuralNet.h"

void neuralNet(char input[INPUT_LEN], short weightsI[WEIGHTS_I_LEN],
               short weightsO[WEIGHTS_O_LEN], int output[OUTPUT_LEN],
               ap_int<SL_LEN> inSwitch, ap_int<SL_LEN> *outLeds)
{
#pragma HLS INTERFACE ap_none port=inSwitch
#pragma HLS INTERFACE ap_none port=outLeds
#pragma HLS INTERFACE s_axilite port=input bundle=BUS
#pragma HLS INTERFACE s_axilite port=weightsI bundle=BUS
#pragma HLS INTERFACE s_axilite port=weightsO bundle=BUS
#pragma HLS INTERFACE s_axilite port=output bundle=BUS
#pragma HLS INTERFACE s_axilite port=return bundle=BUS

    int neurons[NEURONS_LEN];
    int neuron;
    int result;
    int i;
    int j;

    *outLeds = inSwitch;

input_layer:
    for (i = 0; i < NEURONS_LEN; ++i) {
        ap_int<48> accumulator = 0;
    input_dot_product:
        for (j = 0; j < INPUT_LEN; ++j) {
#ifdef NN_PIPELINED
#pragma HLS PIPELINE II=1
#endif
            accumulator += (ap_int<8>)input[j] *
                           (ap_int<16>)weightsI[i * INPUT_LEN + j];
        }
        neuron = (int)(accumulator >> BITS);
        neurons[i] = neuron > 0 ? neuron : 0;
    }

output_layer:
    for (i = 0; i < OUTPUT_LEN; ++i) {
        ap_int<48> accumulator = 0;
    output_dot_product:
        for (j = 0; j < NEURONS_LEN; ++j) {
#ifdef NN_PIPELINED
#pragma HLS PIPELINE II=1
#endif
            accumulator += (ap_int<32>)neurons[j] *
                           (ap_int<16>)weightsO[i * NEURONS_LEN + j];
        }
        result = (int)(accumulator >> BITS);
        output[i] = result;
    }
}

