#ifndef PRODUCER_H
#define PRODUCER_H

#include <systemc.h>
#include "TCPHeader.h"

SC_MODULE(Producer) {

    sc_port<sc_fifo_out_if<TCPHeader*>, 1> out;  //single, bound to one channel

    SC_CTOR(Producer) : seqNum(0) { // init seqNum to zero with constructor
        SC_THREAD(producer_thread);
    }

    void producer_thread();

private:
    unsigned int seqNum;
};

#endif // PRODUCER_H