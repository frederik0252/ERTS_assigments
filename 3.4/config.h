#ifndef ERTS_ASSIGNMENT_1_3_4_CONFIG_H
#define ERTS_ASSIGNMENT_1_3_4_CONFIG_H

#define CHANNEL_BITS 4
#define ERROR_BITS 2
#define DATA_BITS 16
#define MAX_CHANNEL 2
#define CLK_PERIODE 20 // ns

// Figure 5-8 in the assignment uses readyLatency = 1.
#define READY_LATENCY 1

#define NUMBER_OF_TRANSFERS 12
#define OUTPUT_FILE "received_3_4.txt"

static_assert(CHANNEL_BITS >= 2 && CHANNEL_BITS <= 32,
              "Signed channel fields must represent both channels");
static_assert(ERROR_BITS >= 2 && ERROR_BITS <= 32,
              "The example uses error values 0 and 1");
static_assert(DATA_BITS >= 5 && DATA_BITS <= 32,
              "The example data 1..12 must fit in a signed field");
static_assert(MAX_CHANNEL > 0 &&
              MAX_CHANNEL <= (1ULL << (CHANNEL_BITS - 1)),
              "MAX_CHANNEL is the number of channels, numbered from zero");
static_assert(CLK_PERIODE > 0, "Clock period must be positive");
static_assert(READY_LATENCY == 1, "This implementation models latency one");

#endif
