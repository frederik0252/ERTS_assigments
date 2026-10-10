#ifndef ERTS_ZYNQ_TIMER_H
#define ERTS_ZYNQ_TIMER_H

#include "xparameters.h"
#include "xscutimer.h"
#include "xstatus.h"

typedef struct {
    XScuTimer instance;
    u32 start;
} ErtsTimer;

static inline int ErtsTimer_Init(ErtsTimer *timer)
{
#ifdef SDT
    XScuTimer_Config *config =
        XScuTimer_LookupConfig(XPAR_SCUTIMER_BASEADDR);
#else
    XScuTimer_Config *config =
        XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID);
#endif

    if (config == NULL) {
        return XST_FAILURE;
    }
    if (XScuTimer_CfgInitialize(&timer->instance, config, config->BaseAddr) !=
        XST_SUCCESS) {
        return XST_FAILURE;
    }
    XScuTimer_DisableAutoReload(&timer->instance);
    return XST_SUCCESS;
}

static inline void ErtsTimer_Start(ErtsTimer *timer)
{
    XScuTimer_Stop(&timer->instance);
    XScuTimer_LoadTimer(&timer->instance, 0xFFFFFFFFU);
    timer->start = XScuTimer_GetCounterValue(&timer->instance);
    XScuTimer_Start(&timer->instance);
}

static inline u32 ErtsTimer_Stop(ErtsTimer *timer)
{
    const u32 stop = XScuTimer_GetCounterValue(&timer->instance);
    XScuTimer_Stop(&timer->instance);
    return timer->start - stop;
}

static inline void ErtsTimer_DelayTicks(ErtsTimer *timer, u32 ticks)
{
    XScuTimer_Stop(&timer->instance);
    XScuTimer_LoadTimer(&timer->instance, ticks);
    XScuTimer_ClearInterruptStatus(&timer->instance);
    XScuTimer_Start(&timer->instance);
    while (!XScuTimer_IsExpired(&timer->instance)) {
        /* Polling is intentional for this introductory bare-metal exercise. */
    }
    XScuTimer_Stop(&timer->instance);
    XScuTimer_ClearInterruptStatus(&timer->instance);
}

#endif
