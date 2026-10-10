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

The synthesis estimate is 6,612 cycles; the measured VHDL co-simulation
transaction latency is 6,610 cycles.

The native Vitis 2026.1 application was executed on the ZYBO through XSDB. All
ten hardware outputs matched the software reference:

| Target measurement | SCU timer ticks | Time at 325 MHz |
|---|---:|---:|
| ARM software compute | 186,730 | 574.6 us |
| AXI data loading | 233,853 | 719.5 us |
| Accelerator compute | 21,700 | 66.8 us |

The accelerator computation is 8.61x faster than the ARM computation. Including
the 233,853 transfer ticks gives 255,553 ticks per unbatched hardware inference,
which demonstrates that AXI loading is the end-to-end bottleneck.
