#include <systemc.h>

#include "Top.h"

#include <iostream>

int sc_main(int, char *[])
{
    Top top("top");
    sc_start(40 * CLK_PERIODE, SC_NS);

    if (top.slave.received_data.size() != NUMBER_OF_TRANSFERS)
        SC_REPORT_FATAL("test", "Missing or duplicate transfers");
    for (unsigned int i = 0; i < NUMBER_OF_TRANSFERS; ++i)
    {
        if (top.slave.received_data[i] != static_cast<int>(i + 1) ||
            top.slave.received_channels[i] != static_cast<int>(i % MAX_CHANNEL) ||
            top.slave.received_errors[i] != (i == 6 ? 1 : 0))
            SC_REPORT_FATAL("test", "Received payload differs from stimulus");
    }
    std::cout << "PASS: all 12 payloads received exactly once in order.\n";
    return 0;
}
