#include "Slave.h"

#include <iostream>

Slave::Slave(sc_module_name name, const char *output_file)
    : sc_module(name), output_(output_file)
{
    if (!output_)
        SC_REPORT_FATAL(name, "Could not open the slave output file");

    output_ << "time_ns data channel error\n";

    SC_THREAD(drive_ready);
    SC_THREAD(receive);
}

void Slave::drive_ready()
{
    ready.write(SC_LOGIC_0);
    unsigned int cycle = 0;

    while (true)
    {
        wait(clock.negedge_event());

        if (reset.read() != SC_LOGIC_0)
        {
            ready.write(SC_LOGIC_0);
            cycle = 0;
            continue;
        }

        ++cycle;
        // Deterministic backpressure: three cycles low, then two cycles low.
        const bool stalled = (cycle >= 4 && cycle <= 6) ||
                             (cycle >= 11 && cycle <= 12);
        ready.write(stalled ? SC_LOGIC_0 : SC_LOGIC_1);
    }
}

void Slave::receive()
{
    ready_delayed.write(false);
    while (true)
    {
        wait(clock.posedge_event());

        if (reset.read() != SC_LOGIC_0)
        {
            ready_delayed.write(false);
            continue;
        }

        // Read the previous edge's ready before updating the delay register.
        if (valid.read() == SC_LOGIC_1)
        {
            if (!ready_delayed.read())
                SC_REPORT_FATAL(name(), "valid asserted without a prior ready grant");

            received_data.push_back(data.read().to_int());
            received_channels.push_back(channel.read().to_int());
            received_errors.push_back(error.read().to_int());
            output_ << sc_time_stamp().to_seconds() * 1.0e9 << ' '
                    << data.read().to_int() << ' '
                    << channel.read().to_int() << ' '
                    << error.read().to_int() << '\n';
            output_.flush();

            std::cout << name() << ": received data=" << data.read()
                      << ", channel=" << channel.read()
                      << ", error=" << error.read()
                      << " at " << sc_time_stamp() << '\n';
        }

        ready_delayed.write(ready.read() == SC_LOGIC_1);
    }
}
