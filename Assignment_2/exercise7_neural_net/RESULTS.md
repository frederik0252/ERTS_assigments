# HLS verification results

Tool: Vitis HLS 2026.1  
Part: `xc7z010clg400-1`  
Target clock: 10 ns (100 MHz)

Both C simulation and VHDL C/RTL co-simulation pass the self-checking testbench.

| Solution | Latency | BRAM_18K | DSP | FF | LUT |
|---|---:|---:|---:|---:|---:|
| Baseline | 33,056 cycles | 10 | 2 | 638 | 716 |
| Inner loops pipelined (II=1) | 6,612 cycles | 10 | 3 | 1,112 | 1,126 |

Pipelining reduces estimated compute latency by approximately 5x. The tradeoff
is one additional DSP, 474 additional flip-flops, and 410 additional LUTs. BRAM
usage is unchanged. The exported pipelined IP is version 1.0.1 as required by
the assignment.

The measured co-simulation transaction latency is 6,612 cycles. Actual board
measurements must be recorded from the SDK application because AXI transfer
time and processor-side overhead depend on the implemented Vivado design.

