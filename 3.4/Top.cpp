#include "Top.h"

#include <iostream>

Top::Top(sc_module_name name)
    : sc_module(name),
      clock("clock", sc_time(CLK_PERIODE, SC_NS)),
      reset("reset"), ready("ready"), valid("valid"),
      channel("channel"), error("error"), data("data"),
      master("master"), slave("slave", OUTPUT_FILE), trace_file_(nullptr)
{
    master.clock(clock);
    master.reset(reset);
    master.ready(ready);
    master.valid(valid);
    master.channel(channel);
    master.error(error);
    master.data(data);

    slave.clock(clock);
    slave.reset(reset);
    slave.ready(ready);
    slave.valid(valid);
    slave.channel(channel);
    slave.error(error);
    slave.data(data);

    SC_THREAD(generate_reset);

    trace_file_ = sc_create_vcd_trace_file("avalon_st_3_4");
    if (trace_file_ == nullptr)
        SC_REPORT_FATAL(name, "Could not create VCD trace file");
    trace_file_->set_time_unit(1, SC_NS);

    sc_trace(trace_file_, clock, "clock");
    sc_trace(trace_file_, reset, "reset");
    sc_trace(trace_file_, ready, "ready");
    sc_trace(trace_file_, slave.ready_delayed, "ready_delayed");
    sc_trace(trace_file_, valid, "valid");
    sc_trace(trace_file_, channel, "channel");
    sc_trace(trace_file_, error, "error");
    sc_trace(trace_file_, data, "data");
}

Top::~Top()
{
    if (trace_file_ != nullptr)
        sc_close_vcd_trace_file(trace_file_);
}

void Top::generate_reset()
{
    reset.write(SC_LOGIC_1);
    // Release between edges to make startup timing unambiguous.
    wait(2 * CLK_PERIODE + CLK_PERIODE / 4.0, SC_NS);
    reset.write(SC_LOGIC_0);
}
