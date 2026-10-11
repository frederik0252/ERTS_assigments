#include "xparameters.h"
#include "xgpio.h"
#include "led_ip.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "xil_io.h"
#include "sleep.h"
#include "xscutimer.h"

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

            default:
                xil_printf("Key not valid. Please press 1 or 2.\r\n", key);
                break;
        }
    }

    return 0;
}
