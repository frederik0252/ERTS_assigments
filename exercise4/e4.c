#include "xparameters.h"
#include "xgpio.h"
#include "led_ip.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_io.h"
#include "sleep.h"
#include "xscutimer.h"

// 4x4
#define MSIZE 4

typedef union {
	unsigned char comp[MSIZE];
	unsigned int vect;
} VectorType;

typedef VectorType VectorArray[MSIZE];

void setInputMatrices(VectorArray A, VectorArray B)
{
    int dataA[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    int dataB[16] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};
    int row, col;
    int listIndex = 0;

    for (row = 0; row < MSIZE; row++) {
        for (col = 0; col < MSIZE; col++) {
            A[row].comp[col] = dataA[listIndex];
            B[row].comp[col] = dataB[listIndex];
            listIndex++;
        }
    }
}


void displayMatrix(VectorArray input)
{
	int row, col;
	    for (row = 0; row < 4; row++) {
	        for (col = 0; col < 4; col++) {
	            xil_printf("%d\t", input[row].comp[col]);
	        }
	        xil_printf("\r\n");
	    }
}


void multiMatrixSoft(VectorArray A, VectorArray B, VectorArray P)
{
    int row, col, rowA, rowBT, col_index;
    VectorArray BT;

    // Transpose B, switch row and col.
    for (row = 0; row < MSIZE; row++) {
        for (col = 0; col < MSIZE; col++) {
            BT[row].comp[col] = B[col].comp[row];
        }
    }

    //multiplication P = A * BT
    for (rowA = 0; rowA < MSIZE; rowA++) {
        for (rowBT = 0; rowBT < MSIZE; rowBT++) {
            P[rowA].comp[rowBT] = 0; // zero accumulator
            for (col_index = 0; col_index < MSIZE; col_index++)
            {    // accumulate for a rowA index regarding rowBT index with column index
                P[rowA].comp[rowBT] += A[rowA].comp[col_index] * BT[rowBT].comp[col_index];
            }
        }
    }
}


#define ONE_SECOND 325000000

int main(void)
{
    XScuTimer Timer;
    XScuTimer_Config *ConfigPtr;
    XScuTimer *TimerInstancePtr = &Timer;
    int Status;

    // Look up timer config & initialize
    ConfigPtr = XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
    Status = XScuTimer_CfgInitialize(TimerInstancePtr, ConfigPtr, ConfigPtr->BaseAddr);
    if (Status != XST_SUCCESS) {
        xil_printf("Timer init failed\r\n");
        return XST_FAILURE;
    }

    // Enable auto-reload so it continuously counts 1-second blocks
    XScuTimer_EnableAutoReload(TimerInstancePtr);

    // Var
    char key;
    XGpio dip;
    int i, light;
    int count;

    // Initializs Switches
    XGpio_Initialize(&dip, XPAR_SWITCHES_DEVICE_ID);
    XGpio_SetDataDirection(&dip, 1, 0xFFFFFFFF);

    while(1)
    {
        // Menu
        xil_printf("\r\nEnter key:\r\n");
        xil_printf("1 (SW->LEDS)\r\n");
        xil_printf("2 (TIMER->LEDS)\r\n");
        xil_printf("3 (mult)\r\n");

        //read
        do {
            key = inbyte();
        } while (key <= 32 || key > 126);

        switch (key)
        {
            case '1':
                xil_printf("1 Pressed\r\n");

                // Read switches 100 * 100ms = 10s
                for (i = 0; i < 100; i++)
                {
                    light = XGpio_DiscreteRead(&dip, 1);
                    LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, light & 0x0F);
                    usleep(100000); // 100 ms delay
                }
                LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0);

                break;

            case '2':
                xil_printf("2 Pressed\r\n");

                count = 0;

                // Start timer once with Auto-Reload enabled
                XScuTimer_LoadTimer(TimerInstancePtr, ONE_SECOND);
                XScuTimer_Start(TimerInstancePtr);

                // Initial LED state
                LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, count);

                while (count <= 15)
                {
                    // Check if 1 second has elapsed
                    if (XScuTimer_IsExpired(TimerInstancePtr))
                    {
                        // Clear status bit
                        XScuTimer_ClearInterruptStatus(TimerInstancePtr);

                        count++;
                        if (count <= 15) {
                            LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, count);
                            xil_printf("Counter: %d\r\n", count);
                        }
                    }
                }

                XScuTimer_Stop(TimerInstancePtr);
                LED_IP_mWriteReg(XPAR_LED_IP_S_AXI_BASEADDR, 0, 0); // Reset
                break;
				case '3':
				{

					xil_printf("3 Pressed\r\n");
					VectorArray pInst;
					VectorArray aInst;
					VectorArray bInst;

					setInputMatrices(aInst,bInst);

					u32 start_ticks, end_ticks, total_ticks;


					XScuTimer_LoadTimer(TimerInstancePtr, 0xFFFFFFFF);
					XScuTimer_Start(TimerInstancePtr);


					start_ticks = XScuTimer_GetCounterValue(TimerInstancePtr);

					multiMatrixSoft(aInst,bInst,pInst);

					end_ticks = XScuTimer_GetCounterValue(TimerInstancePtr);

					XScuTimer_Stop(TimerInstancePtr);


					total_ticks = start_ticks - end_ticks;


					displayMatrix(pInst);
					xil_printf("Execution Time: %u clock ticks\r\n", total_ticks);


				}
				break;
            default:
                xil_printf("Key not valid. Please press 1 or 2.\r\n", key);
                break;
        }
    }

    return 0;
}

