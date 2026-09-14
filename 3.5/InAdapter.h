#ifndef ERTS_ASSIGNMENT_1_3_5_INADAPTER_H
#define ERTS_ASSIGNMENT_1_3_5_INADAPTER_H

#include <systemc.h>
#include "../3.4/config.h"

// Based on the Exercise 3.5 hint and SmartPitchDetector/InAdapter.h.
// A single SC_THREAD calls write(); the adapter needs no separate process.
template <class T>
class InAdapter : public sc_fifo_out_if<T>, public sc_module
{
public:
    sc_in_clk clock;
    sc_in<sc_logic> reset;
    sc_in<sc_logic> ready;
    sc_out<sc_logic> valid;
    sc_out<sc_int<CHANNEL_BITS>> channel;
    sc_out<sc_int<ERROR_BITS>> error;
    sc_out<sc_int<DATA_BITS>> data;

    explicit InAdapter(sc_module_name name) : sc_module(name)
    {
        valid.initialize(SC_LOGIC_0);
        channel.initialize(0);
        error.initialize(0);
        data.initialize(0);
    }

    void write(const T &value) override
    {
        while (true)
        {
            // Sample ready at edge n. The hint's extra wait before driving
            // data is omitted: this grant applies specifically to edge n+1.
            do
            {
                wait(clock.posedge_event());
            } while (reset.read() != SC_LOGIC_0 ||
                     ready.read() != SC_LOGIC_1);

            data.write(value);
            channel.write(0);
            error.write(0);
            valid.write(SC_LOGIC_1);

            wait(clock.posedge_event()); // The sink samples at edge n+1.
            valid.write(SC_LOGIC_0);

            if (reset.read() == SC_LOGIC_0)
                return; // Blocking write completes only after acceptance.
            // Synchronous reset cancelled the transfer: retry the same value.
        }
    }

private:
    sc_event unused_event_;

    // As in the hint, this adapter only supports blocking writes.
    bool nb_write(const T &) override
    {
        SC_REPORT_FATAL("/InAdapter", "Called unsupported nb_write()");
        return false;
    }

    int num_free() const override
    {
        SC_REPORT_FATAL("/InAdapter", "Called unsupported num_free()");
        return 0;
    }

    const sc_event &data_read_event() const override
    {
        SC_REPORT_FATAL("/InAdapter", "Called unsupported data_read_event()");
        return unused_event_; // Stable lifetime; no allocation/leak.
    }
};

#endif
