#ifndef CONSUMER_H
#define CONSUMER_H

#include <systemc.h>
#include "TCPHeader.h"

SC_MODULE(Consumer) {

    sc_port<sc_fifo_in_if<TCPHeader*>, 1> in;  //single, bound to one channel

    SC_CTOR(Consumer) {
        SC_THREAD(consumer_thread);
    }

    void consumer_thread();
};

#endif // CONSUMER_H
