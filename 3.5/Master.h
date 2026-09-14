#ifndef ERTS_ASSIGNMENT_1_3_5_MASTER_H
#define ERTS_ASSIGNMENT_1_3_5_MASTER_H

#include <systemc.h>
#include "../3.4/config.h"

SC_MODULE(Master)
{
    sc_port<sc_fifo_out_if<int>> out;

    SC_CTOR(Master)
    {
        SC_THREAD(produce);
    }

private:
    void produce();
};

#endif
