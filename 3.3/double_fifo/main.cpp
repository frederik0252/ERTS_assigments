#include <systemc.h>
#include <ctime>
#include "TCPHeader.h"
#include "module1.h"
#include "module2.h"

int sc_main(int argc, char* argv[]) {

    srand((unsigned int)time(0));//init srand

    sc_fifo<TCPHeader*> fifo1(1); // port 1
    sc_fifo<TCPHeader*> fifo2(1); // port 2

    Producer producer("Producer"); //Producer forboth channels
    Consumer consumer1("Consumer1");
    Consumer consumer2("Consumer2");

    producer.out(fifo1);//out of producer
    producer.out(fifo2);//-||-

 
    consumer1.in(fifo1); // in to consumer
    consumer2.in(fifo2);

    sc_start(200, SC_MS);

    return 0;
}