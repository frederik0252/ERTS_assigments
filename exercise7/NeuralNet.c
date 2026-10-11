

#include <stdio.h>
#include <stdlib.h>
#include "xil_printf.h"
#include "XScuTimer.h"
#include "xneuralnet.h"
#include "NeuralNet.h"

#define ONE_SECOND 333000000 // half of the CPU clock speed

XNeuralnet neuralNetHLS;


// Cortex A9 SCU Private Timer Instance
XScuTimer Timer;
// PS Timer related definitions
XScuTimer_Config *ConfigPtr;
XScuTimer *TimerInstancePtr = &Timer;
int CntValue;

void initTimer(void)
{
	int Status;

	// Initialize the timer
	ConfigPtr = XScuTimer_LookupConfig (XPAR_PS7_SCUTIMER_0_DEVICE_ID);
	Status = XScuTimer_CfgInitialize(TimerInstancePtr, ConfigPtr, ConfigPtr->BaseAddr);
	if(Status != XST_SUCCESS){
	  printf("Timer init() failed\r\n");
	}
	// Load timer with delay in multiple of ONE_SECOND
	XScuTimer_LoadTimer(TimerInstancePtr, ONE_SECOND);
	// Set AutoLoad mode
	XScuTimer_EnableAutoReload(TimerInstancePtr);
}

void startTimer(void)
{
	XScuTimer_Start(TimerInstancePtr);
	CntValue = XScuTimer_GetCounterValue(TimerInstancePtr);
}

void stopTimer(char *text)
{
    CntValue = CntValue - XScuTimer_GetCounterValue(TimerInstancePtr);
	XScuTimer_Stop(TimerInstancePtr);

    printf(text, CntValue);
}


// Test neural network with data and weights
signed char inputs[INPUT_LEN];

short weightsIS[WEIGHTS_I_LEN];
float weightsI[WEIGHTS_I_LEN];

short weightsOS[WEIGHTS_O_LEN];
float weightsO[WEIGHTS_O_LEN];

int outputsS[OUTPUT_LEN];
float outputsF[OUTPUT_LEN];

void neuralNetFloat(signed char input[INPUT_LEN], float weightsI[WEIGHTS_I_LEN],
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
		//printf("%0.2f,", neuron);
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
	//printf("\n");
}

void neuralNetHW(signed char input[INPUT_LEN], short wI[WEIGHTS_I_LEN],
		         short wO[WEIGHTS_O_LEN], int output[OUTPUT_LEN])
{
	while (XNeuralnet_IsReady(&neuralNetHLS) == 0); // Check IP core ready

	startTimer();
	XNeuralnet_Write_input_r_Words(&neuralNetHLS, 0, (int*)input, INPUT_LEN/4);
	XNeuralnet_Write_weightsI_Words(&neuralNetHLS, 0, (int*)wI, WEIGHTS_I_LEN/2);
	XNeuralnet_Write_weightsO_Words(&neuralNetHLS, 0, (int*)wO, WEIGHTS_O_LEN/2);
    stopTimer("neuralNet HW load time %d\r\n"); // 1606887 ticks

	startTimer();
	XNeuralnet_Start(&neuralNetHLS);
	while (XNeuralnet_IsDone(&neuralNetHLS) == 0);
	XNeuralnet_Read_output_r_Words(&neuralNetHLS, 0, (int *)output, OUTPUT_LEN);
	stopTimer("neuralNet HW compute time %d\r\n"); // 46908 ticks
}


void testNeuralNet(void)
{
	float diff;
	int i, scale = (1<<BITS);

	for (i=0; i<INPUT_LEN; i++) {
		inputs[i] = i;
	}

	for (i=0; i<WEIGHTS_I_LEN; i++) {
		weightsI[i] = 0.0001*i;
		weightsIS[i] = weightsI[i]*scale + 0.5; // Round and short
	}

	for (i=0; i<WEIGHTS_O_LEN; i++) {
		weightsO[i] = 0.001*i;
		weightsOS[i] = weightsO[i]*scale + 0.5; // Round and short
	}

	// Compute expected results using floats and measure compute time
	startTimer();
	neuralNetFloat(inputs, weightsI, weightsO, outputsF);
	stopTimer("neuralNetFloat CPU time %d\r\n"); // 96684 ticks

	neuralNetHW(inputs, weightsIS, weightsOS, outputsS);

	printf("Scale  	       : %d\n", scale);
	diff = 0;
	for (i=0; i<OUTPUT_LEN; i++) {
		printf("Output[%d]: %d expected %0.2f\n", i, outputsS[i], outputsF[i]);
		diff += abs(outputsS[i] - outputsF[i]);
	}

	if (diff < 10)
		print("Correct");
	else
		print("Wrong");

}


int main()
{
	int depth, bitwidth;


    print("HLS Test Neural Network\n\r");

    initTimer();

    // Initialize the neuralNet driver
    if (XNeuralnet_Initialize(&neuralNetHLS, XPAR_NEURALNET_0_DEVICE_ID) != XST_SUCCESS)
    	return XST_FAILURE;

    depth = XNeuralnet_Get_input_r_Depth(&neuralNetHLS);
    bitwidth = XNeuralnet_Get_input_r_BitWidth(&neuralNetHLS);
    printf("Input depth %d and bit width %d\n\r", depth, bitwidth);

    depth = XNeuralnet_Get_weightsI_Depth(&neuralNetHLS);
    bitwidth = XNeuralnet_Get_weightsI_BitWidth(&neuralNetHLS);
    printf("Input weights depth %d and bit width %d\n\r", depth, bitwidth);

    depth = XNeuralnet_Get_weightsO_Depth(&neuralNetHLS);
    bitwidth = XNeuralnet_Get_weightsO_BitWidth(&neuralNetHLS);
    printf("Output weights depth %d and bit width %d\n\r", depth, bitwidth);

    testNeuralNet();

    return 0;
}
