# ERTS Assignment 2 - ZYBO

This directory contains the source-code deliverables for the mandatory exercises
3, 4, 5, and 7. The generated Vivado projects are intentionally not committed;
use the supplied `Ex3_ZYBO.xpr.zip`, `Ex5.xpr.zip`, and `Ex7.xpr.zip` projects.

## Exercise 3 - UART console

Copy `exercise3_console/src/main.c` into a standalone Zynq application built for
the Ex3 hardware platform. Commands:

- `1`: copy SW0-SW3 to LED0-LED3.
- `2`: count from 0 through 15 on the LEDs, one value per second.
- `help`: show the command list.

## Exercise 4 - software matrix multiplication

Build `exercise4_matrix/src/main.c` together with `common/matrix.c`. The `mult`
command calculates `P = A * B^T`, prints all matrices, and reports elapsed SCU
timer ticks.

## Exercise 5 - matrix hardware accelerator

Build `exercise5_matrix_ip/src/main.c` together with `common/matrix.c` for the
Ex5 hardware platform. The `mult` command benchmarks both the ARM software
implementation and the custom AXI matrix IP and checks that the results match.

The matrix accelerator accepts one packed four-byte row in register 0 and one
packed four-byte row in register 1. Register 2 returns their dot product.

## Exercise 7 - HLS neural network

`exercise7_neural_net/hls` contains the synthesizable two-layer fixed-point
neural network, a self-checking C testbench, and an HLS Tcl script. The script
creates baseline and pipelined solutions for latency/resource comparison.

`exercise7_neural_net/sdk/neuralNetTest.c` loads inputs and weights through the
generated AXI4-Lite driver, compares hardware output with a software fixed-point
reference, and separately measures data-transfer and compute time.

## Host-side verification

Run:

```sh
make test
```

This checks the shared matrix implementation without requiring the Xilinx BSP.

