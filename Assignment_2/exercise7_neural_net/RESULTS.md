# Native verification results - 11 October 2026

Tool: Vitis HLS 2026.1  
Part: `xc7z010clg400-1`  
Target clock: 10 ns (100 MHz)

Both C simulations and pipelined VHDL C/RTL co-simulation pass all five cases:
the explicitly adapted course vector, signed custom vector, zero-state reuse,
negative hidden activations, and signed extrema/wide accumulators. The untouched
provided course reference is preserved and still fails because it encodes a
different weight layout and arithmetic model; see [test provenance](hls/README.md).
Instructor acceptance of that adaptation and of 2026.1 remains unresolved.

| Solution | Latency | BRAM_18K | DSP | FF | LUT |
|---|---:|---:|---:|---:|---:|
| Baseline | 33,056 cycles | 10 | 2 | 638 | 716 |
| Inner loops pipelined (II=1) | 6,612 cycles | 10 | 3 | 1,112 | 1,126 |

Pipelining reduces estimated compute latency by approximately 5x. The tradeoff
is one additional DSP, 474 additional flip-flops, and 410 additional LUTs. BRAM
usage is unchanged. The exported pipelined IP is version 1.0.1 as required by
the assignment.

The synthesis estimate is 6,612 cycles. Five-transaction VHDL co-simulation
reports latency min/average/max of 6,610 / 6,612 / 6,613 cycles. Its observed
transaction intervals include AXI testbench loading gaps; they are not the
II=1 initiation interval of each inner dot-product loop.

The complete system was rebuilt in Vivado 2026.1: WNS +3.462 ns, WHS +0.023 ns,
zero negative slack, 959 LUTs, 1,097 registers, six BRAM tiles and three DSPs.
HLS core estimates and whole-system implementation counts use different scopes.

The native Vitis application was freshly built and its compiler command verified
as `-O2 -g3`. XSDB loaded the bitstream and ELF onto cable `210279655163A`.
All ten custom-vector outputs matched on the initial run and ten subsequent
trials: `[-4, 3, -2, 1, -4, 2, -1, 0, -3, 0]`. The board run uses this one vector;
the broader five-vector arithmetic coverage is from host/HLS/RTL testing.

| Ten subsequent trials | Min ticks | Integer mean ticks | Max ticks | Mean at 325 MHz |
|---|---:|---:|---:|---:|
| ARM software compute | 13,844 | 14,373 | 14,757 | 44.2 us |
| AXI data loading | 228,396 | 228,451 | 228,567 | 702.9 us |
| Accelerator start-to-done | 21,623 | 21,632 | 21,636 | 66.6 us |
| FULL hardware transaction | 253,315 | 253,386 | 253,559 | 779.6 us |

Initial CPU/load/run/FULL measurements were 14,789 / 233,803 / 21,648 / 259,189
ticks. FULL includes packing, input/weight writes, readiness/start/done,
output readback and unpacking; it excludes platform initialization, creation of
test data, result comparison and UART printing. Timer/snapshot overhead is not
subtracted. Repeated trials use the same data with enabled caches; ten samples
are useful observations, not a broad performance characterization.

At these settings the optimized ARM is about **1.51x faster than accelerator
computation alone**, and the full hardware transaction is about **17.63x slower
than CPU compute**. The 100 MHz sequential/pipelined core is not an overall
speedup over this 650 MHz optimized ARM loop. AXI loading dominates the hardware
transaction. Weight retention/batching, streaming/DMA or more parallel hardware
would be separate experiments, not results claimed here.

This supersedes the previous 8.61x compute-only comparison against `-O0` and
the incorrectly labelled load-plus-run "full" latency. The 5x HLS
baseline-to-pipeline improvement remains valid within the same toolchain.

Raw local evidence is under `outputs/fix_evidence/` in the parent workspace:
`ex7_hls_fresh.log`, `ex7_vivado.log`, `ex7_vitis_retry.log`, and
`board_ex7_20261011/{uart.log,xsdb.log,metadata.json}`. The metadata includes the
actual bitstream/ELF hashes. Compiler evidence is also retained in
`hardware/Ex7_vitis/neuralNetTest/build/build_configuration.txt`.
