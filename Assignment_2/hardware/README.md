# Assignment 2 native hardware and applications

These scripts target the original ZYBO XC7Z010 with the installed Vivado/Vitis
2026.1 tools. Exercises 3-5 reuse the supplied Ex5 platform, which contains the
Exercise 3 switch GPIO/custom LED IP plus the matrix accelerator. Exercise 7
uses the saved migrated `Ex7/Ex7.xpr` project and regenerated HLS neural-network
IP. Generated workspaces, bitstreams, and XSAs are ignored by Git.

The course specifies Vivado 2017.2; acceptance of the 2026.1 migration remains
unconfirmed. The user observed and confirmed the full 0..15 LED counter on
11 October 2026. Separate switch-by-switch LED mapping checks remain pending;
build scripts and numerical UART checks do not establish those observations.

All commands below start from the repository root. Run heavy steps sequentially
and keep at most one Vivado GUI open. The Vivado Tcl scripts cap both
`general.maxThreads` and run jobs at two. Set
`CMAKE_BUILD_PARALLEL_LEVEL=2` for the native software builds.

## Build the shared Exercise 3-5 platform

```sh
source /home/firas/AMD/2026.1/Vivado/settings64.sh
vivado -mode batch -source Assignment_2/hardware/build_ex345_2026.tcl -tclargs /home/firas/Downloads/Ex5.xpr.zip
```

The archive argument can point to another copy of the supplied `Ex5.xpr.zip`.
The script leaves it unchanged, extracts the provided block-design Tcl and
custom IP, recreates the design using the current IP catalog, and applies
`ex345_zybo.xdc`. It replaces legacy board-interface selections with explicit
custom GPIO/pin configuration. The original LED and matrix IP implementations
remain the hardware used by the applications.
The extraction records its archive SHA-256 and rejects a changed archive or
an existing unprovenanced extraction. Preserve/rename `generated_ex345` before
switching archives instead of silently mixing old custom IP with new inputs.

Outputs under `Assignment_2/hardware`:

- Project: `generated_ex345/Ex345/Ex345.xpr`
- Bitstream: `generated_ex345/Ex345/Ex345.runs/impl_1/system_wrapper.bit`
- Platform: `Ex345_2026_1.xsa`
- Reports: `ex345_timing_summary.rpt`, `ex345_utilization.rpt`,
  `ex345_drc.rpt`
- Recreated design Tcl: `ex345_system_2026.tcl`

## Build and verify the Exercise 3-5 applications

```sh
source /home/firas/AMD/2026.1/Vitis/settings64.sh
export CMAKE_BUILD_PARALLEL_LEVEL=2
vitis -s Assignment_2/hardware/build_ex345_vitis.py
```

The builder moves an existing `Ex345_vitis` workspace to a timestamped backup,
creates a standalone Cortex-A9 platform from the XSA, and builds these
applications sequentially:

| Exercise | Application | ELF under `Ex345_vitis` |
|---|---|---|
| 3 | `ex3_console` | `ex3_console/build/ex3_console.elf` |
| 4 | `ex4_matrix` | `ex4_matrix/build/ex4_matrix.elf` |
| 5 | `ex5_matrix` | `ex5_matrix/build/ex5_matrix.elf` |

Application compilation uses `-O2`, verified against the compiler-command
database. `ex345_ocm.ld` places heap and all stack modes in low PS on-chip RAM
within `0x00001000..0x0002FFFF`, reserving the first 4 KiB; program code/data
remain in DDR. Linker assertions and post-build ELF inspection check placement,
non-overlap, ARM architecture, and retention of the SDT driver configuration
section. This provides evidence for the course's heap/stack requirement instead
of relying on an IDE setting alone.

Each application build directory receives its map, `elf_header.txt`,
`elf_sections.txt`, `elf_symbols.txt`, and `ocm_verification.json`.
`Ex345_vitis/native_build_evidence.json` records verified artifacts, input
hashes, and compiler commands after successful completion.

Peripheral resolution is semantic: the named switches use AXI GPIO, while the
custom LED IP is written by MMIO. SDT configuration lookup takes base addresses;
the legacy SDK branches take device IDs. Matrix register access handles the
provided IP's `S00_AXI` naming without depending on a generated custom driver.

## Regenerate the Exercise 7 HLS IP and hardware

```sh
source /home/firas/AMD/2026.1/Vitis/settings64.sh
(
  cd Assignment_2/exercise7_neural_net/hls
  vitis-run --mode hls --tcl run_hls.tcl
)
```

The HLS script performs baseline/pipelined C simulation and synthesis, then
pipelined VHDL C/RTL co-simulation and VHDL IP export with requested version
1.0.1. Its combined harness includes the explicitly adapted course case and
additional arithmetic tests. The preserved untouched course test remains a
known failure; see [test provenance](../exercise7_neural_net/hls/README.md).

```sh
source /home/firas/AMD/2026.1/Vivado/settings64.sh
vivado -mode batch -source Assignment_2/hardware/build_ex7_2026.tcl
```

The hardware script uses the new HLS IP repository, upgrades the saved design,
repairs switch/LED port connections, connects both AXI4-Lite interfaces, and
generates:

- Bitstream: `Ex7/Ex7.runs/impl_1/system_wrapper.bit`
- Platform: `Ex7_2026_1.xsa`
- Reports: `ex7_timing_summary.rpt`, `ex7_utilization.rpt`

HLS evidence is under `../exercise7_neural_net/hls/neuralNet_hls`:
`baseline/syn/report`, `pipelined/syn/report`, both `csim/report`
directories, and `pipelined/sim/report/neuralNet_cosim.rpt`.
Read the report corresponding to the current build; older one-vector reports
do not prove the expanded harness passed.

## Build the Exercise 7 native application

```sh
source /home/firas/AMD/2026.1/Vitis/settings64.sh
export CMAKE_BUILD_PARALLEL_LEVEL=2
vitis -s Assignment_2/exercise7_neural_net/sdk/build_vitis_2026.py
```

The builder updates an existing `Ex7_vitis` workspace in place and backs up
changed imported source copies before refreshing them. Its `--rebuild` option
moves the existing workspace to a timestamped backup before clean recreation.
Canonical source remains outside the generated workspace.

The ELF is `Ex7_vitis/neuralNetTest/build/neuralNetTest.elf`. The builder
requests and verifies `-O2` and writes
`Ex7_vitis/neuralNetTest/build/build_configuration.txt` with compiler evidence.
The shared `neuralNet_config.h` is imported; compile-time checks reject stale
driver dimensions, element widths, or unexpected SDT addresses. Input/weight
packing uses aligned word buffers with padded final words.

The current generated SDT table orders the two neural-network AXI addresses
opposite to the driver's named fields. The application explicitly initializes
control BUS at `0x43C00000` and array BUS_r at `0x40000000` and checks that map.
This is a project-specific compatibility correction, not a universal rule for
every HLS IP.

## Run one application on the board

Set the board for USB power and JTAG boot, connect PROG UART, and open its UART
at 115200 baud, 8 data bits, no parity, one stop bit, no flow control. Start one
`hw_server` instance in a separate terminal if none is already running:

```sh
source /home/firas/AMD/2026.1/Vitis/settings64.sh
hw_server -s tcp::3121
```

Use the common runner with one exercise number:

```sh
source /home/firas/AMD/2026.1/Vitis/settings64.sh
xsdb Assignment_2/hardware/run_assignment_on_board.tcl 3 210279655163A
```

Replace `3` with `4`, `5`, or `7` for the corresponding application and use
the connected board's cable serial if different. The runner requires exactly
one matching XC7Z010, resets it, programs the volatile bitstream, initializes
the PS, downloads the matching ELF, and starts execution. Each run replaces the
previous running application; it does not program boot flash.

Exercise 3 accepts `1`, `2`, `mult`, and `help`; Exercises 4 and 5 accept
`mult` and `help`. The literal-transpose matrix fixture should produce rows
containing only 30, 70, 110, and 150, respectively; Exercise 5 additionally
prints software/hardware equality. Physically vary all four switches and
observe the LED pattern, and observe counter changes at one-second intervals.
For Exercise 7, vary switches while its direct LED pass-through is active.
Record these observations separately from numerical output checks.

Exercise 7 runs automatically, checks all outputs, and prints first-run timings
plus min/mean/max over ten subsequent trials. `HW data loading` covers AXI
input/weight writes; `HW compute` covers start-to-done polling. `HW FULL
transaction` additionally includes packing, readiness, output readback, and
unpacking. Only the FULL interval is the measured complete accelerator
transaction; load plus compute alone excludes some work.

## Evidence and current limits

Use generated build manifests, ELF/map files, timing reports, and raw UART
transcripts to identify what passed for a particular source/build/run. Native
captures and review/fix logs are collected in the workspace's sibling
`outputs/native_screenshots`, `outputs/review_evidence`, and
`outputs/fix_evidence` directories. Their dates and build settings matter.

Numerical ANN results are maintained in
[RESULTS.md](../exercise7_neural_net/RESULTS.md). Earlier `-O0` observations are
historical and must not be reused as measurements for the revised `-O2`
benchmark or its new FULL timing boundary. No new timing value is asserted by
these build instructions.

The untouched course-reference discrepancy, instructor acceptance of 2026.1
instead of mandated 2017.2, and individually toggled switch-to-LED mappings remain
explicit acceptance items until corresponding evidence is recorded. The user
confirmed the full physical 0..15 LED counter during the rerun on 11 October 2026.
