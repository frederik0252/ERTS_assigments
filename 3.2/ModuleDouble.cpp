#include "ModuleDouble.h"

void DOUBLE::THREAD_A()
{
    while (true)
    {
        eventA.notify();
        cout << "[THREAD A]: EventA triggered at." << sc_time_stamp() << endl;
        wait(3, SC_MS, eventAack);

        if (eventAack.triggered())
        {
            cout << "[THREAD A]: Recieved Aack:" << sc_time_stamp() << endl;
        }
        else
        {
            cout << "[THREAD A]: Aack Timeout:" << sc_time_stamp() << endl;
        }
    }
}


void DOUBLE::THREAD_B()
{
    while (true)
    {
        eventB.notify();
        cout << "[THREAD B]: EventB triggered at:" << sc_time_stamp() << endl;
        wait(2, SC_MS, eventBack);

        if (eventBack.triggered())
        {
            cout << "[THREAD B]: Recieved Back:" << sc_time_stamp() << endl;
        }
        else
        {
            cout << "[THREAD B]: Back Timeout:" << sc_time_stamp() << endl;
        }
    }
}

void DOUBLE::METHOD_A()
{
    if (Switch)
    {
        cout << "[METHOD A]: EventA received, notifying eventAack:" << sc_time_stamp() << endl;
        eventAack.notify();
        next_trigger(eventB);
        Switch = false;

    }
    else
    {
        cout << "[METHOD A]: EventB received, notifying eventAack:" << sc_time_stamp() << endl;
        eventBack.notify();
        next_trigger(eventA);
        Switch = true;
    }
}