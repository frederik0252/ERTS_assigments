#include <cstdlib>
#include <cstring>
#include "module1.h"

void Producer::producer_thread() {
    while (true) {
        int interval = 2 + rand() % 9; // random 2..10 ms
        wait(interval, SC_MS); 

        TCPHeader* package = new TCPHeader;// create header as allocated memory.
        package->SourcePort = 1234;
        package->DestinationPort = 80;
        package->SequenceNumber = seqNum++;
        package->Acknowledge = 0;
        package->StatusBits = 0;
        package->WindowSize = 0;
        package->Checksum = 0;
        package->UrgentPointer = 0;
        memset(package->Data, 0, DATA_SIZE); // fill data with zeros.

        cout << name()
            << " (producer_thread): sending package, SequenceNumber = "
            << package->SequenceNumber << " at time:" << sc_time_stamp() << endl;

        for (int i = 0; i < out.size(); i++) {
            TCPHeader* copy = new TCPHeader(*package); // copy 
            out[i]->write(copy);// write to copy.
        }
        delete package; // delete original
    }
}