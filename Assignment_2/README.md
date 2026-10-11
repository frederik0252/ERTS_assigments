# ERTS Assignment 2 - ZYBO

This directory contains the mandatory Exercise 3, 4, 5, and 7 implementations,
host checks, native-build scripts, and the saved Exercise 7 Vivado project.
Build and board-run instructions are in [hardware/README.md](hardware/README.md).
The [hand-in package](handin/README.md) contains the final PDFs, their LaTeX
sources, referenced native screenshots, and retained verification evidence.

The course handout explicitly specifies Vivado 2017.2. This work uses the
installed Vivado/Vitis 2026.1 tools; acceptance of that tool-version deviation
remains an external course decision. The user confirmed the physical 0..15
LED counter at approximately one step/second on 11 October 2026. Separate
switch-to-LED mapping observations remain a walkthrough check.

## Exercise 3 - UART console

`exercise3_console/src/main.c` runs on the migrated course Ex5 hardware platform,
which includes the switch GPIO and custom LED IP needed by Exercise 3 as well as
the matrix IP needed by Exercise 5. Commands are:

- `1`: copy SW0-SW3 to LED0-LED3 and print the switch value.
- `2`: count from 0 through 15 on the LEDs, one step per second.
- `mult`: run the Exercise 4 software matrix multiplication.
- `help`: show the command list.

Switches use the named AXI GPIO instance. LEDs use register 0 of the supplied
custom `led_ip`; they are not treated as a second AXI GPIO device. The counter
uses a continuously reloading SCU timer so UART output is not added to each
one-second period. Heap and stack are placed in low PS on-chip memory by the
native linker template and verified from each generated ELF.

## Exercise 4 - software matrix multiplication

The `mult` command is available in the Exercise 3 console and the separate
`exercise4_matrix/src/main.c` application. Shared arithmetic is in
`common/matrix.c`; UART formatting is in `common/matrix_uart.c`. The required
global instances are `pInst`, `aInst`, and `bTInst`.

The fixture follows the literal transpose printed on assignment page 6:
`bTInst` has four rows of `[1, 2, 3, 4]`. Multiplication follows the page 7
row-dot-product formula, `P[row][col] = sum(A[row][k] * bTInst[col][k])`.
The expected result has constant rows of 30, 70, 110, and 150 respectively.
The program prints the inputs, result, and software execution time in SCU timer
ticks. This replaces the earlier row-constant stored fixture.

## Exercise 5 - matrix hardware accelerator

`exercise5_matrix_ip/src/main.c` provides `mult` to run both software and
hardware multiplication, print both results and timing intervals, and compare
them. The supplied matrix IP takes packed four-byte rows in registers 0 and 1
and returns the dot product's low byte in register 2. Software deliberately
matches that modulo-256 arithmetic; it is not an arbitrary-precision matrix
product.

`common/zynq_peripherals.h` resolves semantic peripheral names. Modern SDT
GPIO/timer lookups use base addresses; legacy SDK lookups use device IDs.
Matrix addressing supports both the packaged `S00_AXI` name and the handout's
`S_AXI` spelling. No numeric GPIO index is assumed to identify the switches.

## Exercise 7 - HLS neural network

`exercise7_neural_net/hls` contains the fixed-point accelerator, baseline and
pipelined HLS flow, and combined testbench. The C-compatible
`neuralNet_config.h` supplies dimensions to HLS and the target application.
The SDK build checks them against generated driver depths and widths, so
changing dimensions also requires regenerating the hardware and BSP.

The untouched course testbench and header are preserved under
`hls/provided_course`. Its original floating reference uses overlapping weight
row strides and still fails against the dense-layer implementation. The adapted
course test preserves the supplied input and weight generation but uses the
corrected strides, required integer shifts, and ReLU. Additional tests cover
signed inputs, zero inputs, negative hidden activations, wide accumulators, and
LED values. See [HLS test provenance](exercise7_neural_net/hls/README.md) for the
exact distinction; passing adapted tests does not claim the untouched original
passed. Course acceptance of that reference correction remains to be confirmed.

`exercise7_neural_net/sdk/neuralNetTest.c` checks software/hardware outputs and
reports a first run plus minimum, mean, and maximum over ten subsequent trials.
The native builder requests and verifies `-O2`. Timing labels distinguish:

- CPU reference computation.
- AXI input/weight writes.
- Accelerator start-to-done polling.
- FULL hardware transaction: packing, writes, readiness, execution, output
  readback, and unpacking.

Loading plus compute alone is not the full transaction. Historical `-O0`
measurements must not be presented as results for the revised `-O2` program.
Recorded results and their context belong in
[exercise7_neural_net/RESULTS.md](exercise7_neural_net/RESULTS.md).

## Host-side verification

Run from the repository root:

```sh
make -C Assignment_2 test
bash Assignment_2/exercise7_neural_net/hls/test_host.sh /home/firas/AMD/2026.1/Vitis/include
```

The matrix test checks the fixture, independent dot-product results, and
overflow behavior without a Xilinx BSP. The neural-network script runs the
adapted/custom tests at 196/32/10 and 8/3/4 dimensions, then explicitly reproduces
the preserved original test's known failure. It prints raw-log locations.
Host tests do not establish RTL timing, board execution, or physical LED state;
use the separate native reports and UART/observation evidence for those claims.
