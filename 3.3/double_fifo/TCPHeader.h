#ifndef TCPHEADER_H
#define TCPHEADER_H

#include <systemc.h>

#define PACKET_SIZE 512
#define DATA_SIZE (PACKET_SIZE - 20)

typedef struct {
    sc_uint<16> SourcePort;
    sc_uint<16> DestinationPort;
    sc_uint<32> SequenceNumber;
    sc_uint<32> Acknowledge;
    sc_uint<16> StatusBits;
    sc_uint<16> WindowSize;
    sc_uint<16> Checksum;
    sc_uint<16> UrgentPointer;
    char        Data[DATA_SIZE];
} TCPHeader;

#endif // TCPHEADER_H