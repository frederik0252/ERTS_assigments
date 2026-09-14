#ifndef ERTS_ASSIGNMENT_1_3_4_MASTER_H
#define ERTS_ASSIGNMENT_1_3_4_MASTER_H

#include <systemc.h>

#include "config.h"

SC_MODULE(Master)
{
    sc_in_clk clock;
    sc_in<sc_logic> reset;

    sc_in<sc_logic> ready;
    sc_out<sc_logic> valid;
    sc_out<sc_int<CHANNEL_BITS>> channel;
    sc_out<sc_int<ERROR_BITS>> error;
    sc_out<sc_int<DATA_BITS>> data;

    SC_CTOR(Master)
    {
        SC_THREAD(transmit);
    }

private:
    void transmit();
};

#endif
