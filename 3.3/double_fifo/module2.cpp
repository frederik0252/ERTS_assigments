#include "module2.h"

void Consumer::consumer_thread() {
    while (true) {
        TCPHeader* package = in->read();

        cout << name()
            << " (consumer_thread): received package, SequenceNumber = "
            << package->SequenceNumber << " at time:" << sc_time_stamp() << endl;

       delete package; // created a copy to fix memory leak.

        // NOT deleted here to avoid a double free. Memory leak but okay as long as simulation time is short.
    }
}