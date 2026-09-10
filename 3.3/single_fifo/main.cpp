#include <systemc.h>
#include <ctime>
#include "TCPHeader.h"
#include "module1.h"
#include "module2.h"

int sc_main(int argc, char* argv[]) {

    srand((unsigned int)time(0)); //init srand

    sc_fifo<TCPHeader*> fifo(1); // room for four header pointers 

    Producer producer("Producer");
    Consumer consumer("Consumer");

    producer.out(fifo);//out of producer
    consumer.in(fifo); // in to consumer

    sc_start(200, SC_MS);

    return 0;
}
