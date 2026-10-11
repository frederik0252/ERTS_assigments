#ifndef ERTS_NEURAL_NET_CONFIG_H
#define ERTS_NEURAL_NET_CONFIG_H

/* Shared by HLS C++ and the C application. Override before synthesis only. */
#ifndef SL_LEN
#define SL_LEN 4
#endif
#ifndef BITS
#define BITS 15
#endif
#ifndef INPUT_LEN
#define INPUT_LEN 196
#endif
#ifndef NEURONS_LEN
#define NEURONS_LEN 32
#endif
#ifndef OUTPUT_LEN
#define OUTPUT_LEN 10
#endif

#define WEIGHTS_I_LEN (INPUT_LEN * NEURONS_LEN)
#define WEIGHTS_O_LEN (NEURONS_LEN * OUTPUT_LEN)

#if INPUT_LEN < 1 || NEURONS_LEN < 1 || OUTPUT_LEN < 1
#error "Neural-network dimensions must be positive"
#endif
#if BITS != 15
#error "This assignment implementation uses signed Q15 weights"
#endif
#if SL_LEN < 1 || SL_LEN > 31
#error "The switch/LED width must be between 1 and 31"
#endif

#endif
