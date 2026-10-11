

#define SL_LEN 4								// Length of switches and leds
#define BITS 15							        // Weights shifted number of bits (*2^BIT_SHIFT)
#define INPUT_LEN 196               			// Number of inputs 28/2*28/2 MINST pixels (49)
#define NEURONS_LEN  32               			// Number of neurons (16)
#define WEIGHTS_I_LEN (INPUT_LEN*NEURONS_LEN)  	// Number of weights in input layer
#define OUTPUT_LEN 10			        		// Number of outputs 10 digits
#define WEIGHTS_O_LEN (NEURONS_LEN*OUTPUT_LEN)  // Number of weights in output layer

