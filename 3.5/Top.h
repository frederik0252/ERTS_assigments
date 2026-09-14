#ifndef ERTS_ASSIGNMENT_1_3_5_TOP_H
#define ERTS_ASSIGNMENT_1_3_5_TOP_H

#include <systemc.h>
#include "Master.h"
#include "InAdapter.h"
#include "../3.4/Slave.h"

SC_MODULE(Top)
{
    sc_clock clock;
    sc_signal<sc_logic> reset, ready, valid;
    sc_signal<sc_int<CHANNEL_BITS>> channel;
    sc_signal<sc_int<ERROR_BITS>> error;
    sc_signal<sc_int<DATA_BITS>> data;

    Master master;
    InAdapter<int> inAdapt;
    Slave slave;

    SC_CTOR(Top);
    ~Top() override;

private:
    sc_trace_file *trace_file_;
    void generate_reset();
};

#endif
