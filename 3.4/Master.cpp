#include "Master.h"

#include <iostream>

void Master::transmit()
{
    valid.write(SC_LOGIC_0);
    channel.write(0);
    error.write(0);
    data.write(0);

    unsigned int next_word = 0;
    unsigned int accepted_words = 0;
    while (true)
    {
        wait(clock.posedge_event());
        if (reset.read() != SC_LOGIC_0)
        {
            valid.write(SC_LOGIC_0);
            next_word = accepted_words; // Retry a reset-cancelled word.
            continue;
        }

        // The old valid/data values are being sampled by the sink now.
        if (valid.read() == SC_LOGIC_1)
        {
            ++accepted_words;
            std::cout << name() << ": transferred data=" << data.read()
                      << " at " << sc_time_stamp() << '\n';
        }

        // ready sampled at edge n grants the transfer at edge n+1.
        // Signal writes become visible after this edge's evaluation phase.
        if (ready.read() == SC_LOGIC_1 &&
            next_word < NUMBER_OF_TRANSFERS)
        {
            data.write(next_word + 1); // Send 1, 2, 3, ..., 12.
            channel.write(static_cast<int>(next_word % MAX_CHANNEL));
            error.write(next_word == 6 ? 1 : 0);
            valid.write(SC_LOGIC_1);
            ++next_word;
        }
        else
        {
            valid.write(SC_LOGIC_0);
        }
    }
}
