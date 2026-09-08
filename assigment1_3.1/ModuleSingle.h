#pragma once
#ifndef MODULE_SINGLE_H
#define MODULE_SINGLE_H

#include <systemc.h>

SC_MODULE(SINGLE) {
    sc_event e_trigger;
    SC_CTOR(SINGLE) {
        SC_THREAD(my_thread_process);
        

        SC_METHOD(my_method_process);
        sensitive << e_trigger;
    }

    void my_method_process(void);
    void my_thread_process(void);
    sc_uint<4> counter;
   
};

#endif