#include "Master.h"
#include <iostream>

void Master::produce()
{
    for (int value = 1; value <= NUMBER_OF_TRANSFERS; ++value)
    {
        out->write(value);
        std::cout << name() << ": write(" << value << ") returned at "
                  << sc_time_stamp() << '\n';
    }
}
