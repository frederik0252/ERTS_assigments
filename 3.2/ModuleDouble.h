#pragma once
#ifndef MODULE_DOUBLE_H
#define MODULE_DOUBLE_H

#include <systemc.h>




SC_MODULE(DOUBLE) {

    sc_event eventA, eventB;
    sc_event eventAack, eventBack;

    bool Switch;
    
    SC_CTOR(DOUBLE) {
        SC_THREAD(THREAD_A);
        SC_THREAD(THREAD_B);

        SC_METHOD(METHOD_A);
        sensitive << eventA; 
        dont_initialize();    
    }

    void THREAD_A(void);
    void THREAD_B(void);
    void METHOD_A(void);
   
};

#endif