#include "Master.h"
#include <fstream>
#include <iostream>
#include <vector>

SC_MODULE(FifoConsumer)
{
    sc_fifo_in<int> in;
    std::vector<int> received;

    SC_CTOR(FifoConsumer)
    {
        SC_THREAD(consume);
    }

    void consume()
    {
        std::ofstream output("received_tlm.txt");
        if (!output)
            SC_REPORT_FATAL(name(), "Could not open the TLM output file");
        output << "time_ns data\n";
        for (unsigned int i = 0; i < NUMBER_OF_TRANSFERS; ++i)
        {
            const int value = in.read();
            received.push_back(value);
            output << sc_time_stamp().to_seconds() * 1.0e9
                   << ' ' << value << '\n';
        }
    }
};

int sc_main(int, char *[])
{
    Master master("master");
    FifoConsumer consumer("consumer");
    sc_fifo<int> fifo("fifo", 4);
    master.out(fifo);
    consumer.in(fifo);
    sc_start(); // Untimed FIFO communication settles in delta cycles.

    if (consumer.received.size() != NUMBER_OF_TRANSFERS)
        SC_REPORT_FATAL("test", "Missing TLM words");
    for (unsigned int i = 0; i < NUMBER_OF_TRANSFERS; ++i)
        if (consumer.received[i] != static_cast<int>(i + 1))
            SC_REPORT_FATAL("test", "TLM value or ordering mismatch");
    std::cout << "PASS: untimed FIFO delivered the same 12 data values.\n";
    return 0;
}
