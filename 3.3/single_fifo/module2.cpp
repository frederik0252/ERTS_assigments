#include "module2.h"

void Consumer::consumer_thread() {
    while (true) {
        TCPHeader* package = in->read(); //read package from sc_fifo_in

        cout << name()
            << " (consumer_thread): received package, SequenceNumber = "
            << package->SequenceNumber << " at time:" << sc_time_stamp() << endl;

        delete package;// delete allocated memory to avoid memory leak
    }
}