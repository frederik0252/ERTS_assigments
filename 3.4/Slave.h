#ifndef ERTS_ASSIGNMENT_1_3_4_SLAVE_H
#define ERTS_ASSIGNMENT_1_3_4_SLAVE_H

#include <systemc.h>

#include <fstream>
#include <vector>

#include "config.h"

class Slave : public sc_module
{
public:
    sc_in_clk clock;
    sc_in<sc_logic> reset;

    sc_out<sc_logic> ready;
    sc_in<sc_logic> valid;
    sc_in<sc_int<CHANNEL_BITS>> channel;
    sc_in<sc_int<ERROR_BITS>> error;
    sc_in<sc_int<DATA_BITS>> data;

    // Internal delayed-ready signal, also traced for explanation/testing.
    sc_signal<bool> ready_delayed{"ready_delayed"};
    std::vector<int> received_data;
    std::vector<int> received_channels;
    std::vector<int> received_errors;

    SC_HAS_PROCESS(Slave);
    Slave(sc_module_name name, const char *output_file = OUTPUT_FILE);

private:
    std::ofstream output_;

    void drive_ready();
    void receive();
};

#endif
