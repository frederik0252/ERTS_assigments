# Exploratory HLS Evaluation: GEMM vs. LayerNorm for RF-DETR Nano

**Target Hardware:** Digilent Zybo (Rev B) — Xilinx Zynq-7010 (`xc7z010clg400-1`)  
**HLS Tool & Synthesis Proxy:** Vivado HLS `2017.2` (synthesized on `xc7z020clg484-1` at `10.00 ns` / `100 MHz`, evaluated manually against `z010` limits)  

---

## 1. Executive Summary

This report documents an exploratory High-Level Synthesis (HLS) feasibility study comparing two core transformer/vision-backbone operators—**General Matrix Multiplication (GEMM)** and **Layer Normalization (LayerNorm)**—for hardware acceleration of **RF-DETR Nano** on a **Digilent Zybo Rev B** (`xc7z010clg400-1`).

* **Primary Candidate Selected:** **GEMM (`gemm_hls`)**
* **Key Findings:**
  1. **Software Bottleneck:** In the software baseline, GEMM accounts for **`111.414 ms`** of execution time versus **`7.675 ms`** for LayerNorm (~14.5$\times$ heavier workload), making GEMM a far more impactful target for Amdahl's Law speedup.
  2. **Board BRAM Feasibility:** Storing full un-tiled token tensors (`256 × 192` floats) inside on-chip AXI4-Lite BRAMs for LayerNorm required **`260` BRAM_18K** blocks, exceeding the Zybo Rev B's hard limit of **`60` BRAM_18K** by more than 4$\times$.
  3. **Optimization Headroom:** Both kernels were evaluated as **unoptimized sequential baselines** (no `PIPELINE`, `UNROLL`, or `ARRAY_PARTITION` pragmas). GEMM's baseline (`8` BRAM_18K, `8` DSP48E) leaves substantial headroom within the Zybo Rev B budget (`60` BRAM_18K, `80` DSP48E) to explore tiling and loop pipelining.

---

## 2. Why GEMM and LayerNorm Were Chosen as Candidates

In transformer-based object detectors like **RF-DETR Nano**, execution time is dominated by two fundamentally different computational patterns:

1. **GEMM (Compute-Bound / Dense Linear Projections):**
   * Used throughout attention projections ($Q, K, V$, output projection) and Feed-Forward Networks (FFNs).
   * Exhibits high arithmetic intensity ($O(M \cdot N \cdot K)$ multiply-accumulate operations) and regular data reuse, making it a classic candidate for FPGA DSP parallelism and systolic/tiled acceleration.
   * **Software Baseline Runtime:** `111.414 ms`.

2. **LayerNorm (Memory- and Reduction-Bound / Non-Linear Elementwise Normalization):**
   * Applied across embedding dimensions per token (`mean`, `variance`, inverse square root, and affine $\gamma / \beta$ scaling).
   * Requires multiple passes over each token vector plus transcendental/division operations (`sqrt`, floating-point division) that are expensive in hardware and serialize execution across reduction dependencies.
   * **Software Baseline Runtime:** `7.675 ms`.

Comparing both allowed us to evaluate whether accelerating the heavy compute kernel (GEMM) or offloading the multi-pass normalization kernel (LayerNorm) is more viable under strict Zynq-7010 resource constraints.

---

## 3. Target Hardware Constraints (Digilent Zybo Rev B)

Physical inspection of the target board confirmed it is the **original Digilent Zybo (Rev B)** housing the **`xc7z010clg400-1`** SoC. Because the `xc7z010` device support files were not installed in the active Vivado HLS `2017.2` environment, synthesis was executed using `xc7z020clg484-1` at a `10.00 ns` (`100 MHz`) clock target, and all utilization metrics were evaluated manually against the true `xc7z010` limits:

| Resource | Zybo Rev B (`xc7z010clg400-1`) Limit | Synthesis Proxy (`xc7z020clg484-1`) |
| :--- | :---: | :---: |
| **BRAM_18K** | **60** | 280 |
| **DSP48E** | **80** | 220 |
| **LUT** | **17,600** | 53,200 |
| **FF** | **35,200** | 106,400 |
| **Target Clock** | **10.00 ns (100 MHz)** | 10.00 ns (100 MHz) |

---

## 4. Methodology & Dimension Selection

### 4.1 Baseline Architecture (Unoptimized Sequential Reference)
To establish a clean, functionally verified baseline before introducing area-for-speed trade-offs:
* Both `gemm_hls` and `layernorm_hls` were written in standard C++ using single-precision `float` arithmetic.
* Only `s_axilite` interface pragmas (for control and buffer mapping) and `LOOP_TRIPCOUNT` pragmas (for latency estimation) were applied.
* **No parallelization pragmas** (`#pragma HLS PIPELINE`, `#pragma HLS UNROLL`, `#pragma HLS ARRAY_PARTITION`, or `DATAFLOW`) were applied yet.

### 4.2 Chosen Dimensions & Rationale
Because exact layer-by-layer ONNX/PyTorch tensor dimensions for the specific RF-DETR Nano checkpoint were not yet extracted during this exploratory phase, representative working bounds were chosen per kernel:

* **GEMM (`gemm_hls`):**
  * **Chosen Bounds:** `max = 32` in `LOOP_TRIPCOUNT` (`16 × 16 × 16` matrix workload in C/RTL co-simulation).
  * **Rationale:** Scaled down intentionally to enable rapid C synthesis and cycle-accurate `xsim` RTL co-simulation while establishing a baseline resource-per-MAC profile (`8` BRAM_18K, `8` DSP48E).
* **LayerNorm (`layernorm_hls`):**
  * **Chosen Bounds:** `MAX_TOKENS = 256`, `MAX_DIM = 192` ($256 \times 192 = 49,152$ elements per input/output tensor).
  * **Rationale:** Chosen to stress-test full un-tiled token-tensor buffering on-chip (`input` and `output` stored completely in AXI4-Lite BRAMs) and immediately expose memory capacity bottlenecks against the Zybo Rev B's `60` BRAM_18K ceiling.

*(Note: Because the two kernels were synthesized with different dimension bounds, resource comparisons must account for buffer sizing rather than treating raw numbers as identical matrix shapes.)*

---

## 5. Experimental Results

### 5.1 Quantitative Comparison Table

| Metric | Software Baseline | `gemm_hls` (Unoptimized, `max=32`) | `layernorm_hls` (Unoptimized, `256×192`) |
| :--- | :---: | :---: | :---: |
| **Software Runtime** | `111.414 ms` (GEMM) / `7.675 ms` (LN) | — | — |
| **Estimated Clock (`10.00 ns` target)** | — | `8.27 ns` (Pass) | `8.65 ns` (Pass) |
| **Synthesis Latency (Cycles)** | — | `16` min / `330,817` max | `117` min / `2,179,846` max |
| **C/RTL Co-Simulation** | — | **PASS** (`87,002` cycles for $16^3$) | Not completed (slow `256×192` RTL sim) |
| **BRAM_18K Used** | — | **`8`** (`13.3%` of Zybo Rev B) | **`260`** (**`433.3%` of Zybo Rev B — EXCEEDS**) |
| **DSP48E Used** | — | **`8`** (`10.0%` of Zybo Rev B) | **`6`** (`7.5%` of Zybo Rev B) |
| **FF Used** | — | `1,706` (`4.8%` of Zybo Rev B) | `3,744` (`10.6%` of Zybo Rev B) |
| **LUT Used** | — | `1,643` (`9.3%` of Zybo Rev B) | `4,470` (`25.4%` of Zybo Rev B) |

### 5.2 Key Observations
1. **Timing Closure:** Both designs cleanly met the `100 MHz` (`10.00 ns`) timing requirement (`8.27 ns` for GEMM and `8.65 ns` for LayerNorm).
2. **LayerNorm Memory Bottleneck:** Buffering full `256 × 192` `float` input and output tensors via `s_axilite` memory interfaces required `260` BRAM_18K blocks—more than **4$\times$ the total BRAM available** on the Zybo Rev B (`60` BRAM_18K). Making LayerNorm fit on `xc7z010` would require architectural restructuring (row-by-row streaming, DDR bursts via `m_axi`, or drastically smaller on-chip buffers), for a kernel that only takes `7.675 ms` in software.
3. **GEMM Viability & Scaling Headroom:** At the baseline scale, GEMM uses only `13.3%` of the Zybo Rev B's BRAM (`8/60`) and `10.0%` of its DSPs (`8/80`), while targeting a `111.414 ms` software bottleneck. When scaling GEMM to full RF-DETR Nano dimensions, on-chip BRAM must still be managed carefully (e.g., via loop tiling or `m_axi` DDR transfers so matrices do not exceed `60` BRAM_18K), while the remaining `72` DSP48E slices provide ample room for loop pipelining and partial unrolling.

---

## 6. Conclusion & Next Steps

**Conclusion:** **GEMM (`gemm_hls`)** is selected as the primary acceleration candidate for the Digilent Zybo Rev B. It addresses the dominant `111.414 ms` software workload and fits comfortably within the `xc7z010` resource envelope when paired with proper memory tiling.

**Recommended Next Steps:**
1. Extract the exact `(M, N, K)` GEMM layer dimensions from the RF-DETR Nano model/profiling script.
2. Design a **tiled GEMM architecture** (using `m_axi` for external DDR storage and small on-chip BRAM tiles capped well below `60` BRAM_18K).
3. Apply `#pragma HLS PIPELINE` and `#pragma HLS ARRAY_PARTITION` to the inner compute tile to maximize DSP48E utilization (targeting up to ~60–70 of the `80` available DSPs) and measure speedup against the `111.414 ms` software baseline.