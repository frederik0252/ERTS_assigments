#include <stdio.h>
#include "neuralNet.h"

void annFloat(char input[INPUT_LEN], float weightsI[WEIGHTS_I_LEN],
			  float weightsO[WEIGHTS_O_LEN], float output[OUTPUT_LEN])
{
	float neurons[NEURONS_LEN];
	float neuron, result;
	int i, j, offsetI, offsetO;

	// Compute input layer
	for (i=0; i<NEURONS_LEN; i++) {
		neuron = 0;
		offsetI = NEURONS_LEN * i;
		// Compute neurons
		for (j=0; j<INPUT_LEN; j++) {
			neuron += input[j] * weightsI[j+offsetI];
		}
		printf("%0.2f,", neuron);
		neurons[i] = neuron;
		if (neuron < 0) // ReLU
			neurons[i] = 0;
	}

	// Compute output layer
	for (i=0; i<OUTPUT_LEN; i++) {
		result = 0;
		offsetO = OUTPUT_LEN * i;
		for (j=0;  j<NEURONS_LEN; j++) {
			result += neurons[j] * weightsO[j+offsetO];
		}
		output[i] = result;
	}
	printf("\n");
}

int main()
{
	ap_int<SL_LEN> sw=0x7;
	ap_int<SL_LEN> leds;

	int i, ledRes;
	char inputs[INPUT_LEN];

	short weightsIS[WEIGHTS_I_LEN];
	float weightsI[WEIGHTS_I_LEN];

	short weightsOS[WEIGHTS_O_LEN];
	float weightsO[WEIGHTS_O_LEN];

	int outputsS[OUTPUT_LEN];
	float outputs[OUTPUT_LEN];
	float diff;
	int scale = (1<<BITS);

	for (i=0; i<INPUT_LEN; i++) {
		inputs[i] = i%255;
	}

	for (i=0; i<WEIGHTS_I_LEN; i++) {
		weightsI[i] = 0.0001*i;
		weightsIS[i] = weightsI[i]*scale + 0.5;
	}

	for (i=0; i<WEIGHTS_O_LEN; i++) {
		weightsO[i] = 0.001*i;
		weightsOS[i] = weightsO[i]*scale + 0.5;
	}

	annFloat(inputs, weightsI, weightsO, outputs);

	neuralNet(inputs, weightsIS, weightsOS, outputsS, sw, &leds);

	printf("Scale  	       : %d\n", scale);
	ledRes = leds;
	printf("Leds           : %d\n", ledRes);
	diff = 0;
	for (i=0; i<OUTPUT_LEN; i++) {
		printf("Output[%d]: %d expected %f\n", i, outputsS[i], outputs[i]);
		diff += abs(outputsS[i] - outputs[i]);
	}

	if (diff < 10)
		return 0;
	else
		return 1;

}
