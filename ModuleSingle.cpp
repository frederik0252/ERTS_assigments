#include "ModuleSingle.h"

void SINGLE::my_thread_process()
{
    while (true)
    {
        e_trigger.notify();   
        wait(2, SC_MS);        
    }
}

void SINGLE::my_method_process()
{
	counter = counter + 1;
    std::cout << "Counter value: " << counter << " at " << sc_time_stamp() << std::endl;
}