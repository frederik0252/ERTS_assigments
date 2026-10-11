# Neural-network test provenance

`provided_course/testbench.cpp` and `provided_course/neuralNet.h` are untouched
copies of the supplied `Assignment_2/Ex7ANN` files. They are retained as evidence,
not silently substituted or described as passing. With the standard dense-layer
implementation, the original test exits 1: its first output is 331 versus the
original floating reference's 41.003418; its last is 4805 versus 201.729141.

The supplied reference advances input-weight rows by `NEURONS_LEN` and
output-weight rows by `OUTPUT_LEN`. Correct fully connected rows advance by
`INPUT_LEN` and `NEURONS_LEN` respectively. It also keeps hidden activations as
floating-point values, unlike the assignment's required integer shift after
each dot product. These are mathematical/test-reference corrections, not
Vivado-version compatibility changes. The literal supplied-testbench acceptance
discrepancy remains visible and should be discussed with the course instructor.
The original harness can pass a different implementation using those
overlapping row offsets; therefore this is a real specification/reference
acceptance decision, not merely a cosmetic test warning.

`course_testbench.cpp` preserves the course input sequence (`i % 255`), floating
weight generation (`0.0001*i`, `0.001*i`), and positive Q15 rounding. It checks
those quantized inputs and weights against an adapted integer reference:
contiguous dense rows, explicit signed input decoding, floor division by 2^15
after each dot product, and hidden-layer ReLU. `test_reference.h` uses 64-bit
host arithmetic and traverses the weight arrays sequentially. It computes
expected values rather than substituting hard-coded passing outputs.

`testbench.cpp` runs the adapted course case plus the original custom signed
vector, zero inputs after a nonzero inference, negative hidden activations, and
signed input/weight extremes. Each invocation checks every output and the LED
value; switch patterns include 0, 5, 7, 10, and 15. These simulation checks do not
replace physical switch/LED observation.

Run lightweight host checks with the installed HLS headers:

```sh
bash Assignment_2/exercise7_neural_net/hls/test_host.sh /path/to/Vitis/include
```

The script checks the default 196/32/10 dimensions and an 8/3/4 configuration,
then separately reproduces and labels the original course test's failure. It
prints the directory holding raw logs and test binaries. Success of this script
means the adapted/custom checks passed and the known original mismatch was
reproduced; it does not mean the untouched original passed.

`neuralNet_config.h` is the shared C-compatible dimension configuration for HLS
and the target application. Dimensions can be overridden before synthesis, for
example with `-DINPUT_LEN=8 -DNEURONS_LEN=3 -DOUTPUT_LEN=4`. Hardware integration,
the generated driver, and the target application must all be regenerated for a
changed configuration. Alternate-dimension host checks alone do not prove RTL
synthesis, timing, or execution for that configuration.

`run_hls.tcl` runs the same combined testbench in both baseline and pipelined C
simulation, then in pipelined VHDL C/RTL co-simulation. The accelerator arithmetic
and pipeline directives are unchanged by this test expansion. The script anchors
its working directory and preserves any previous generated project under
`neuralNet_hls.backup-<UTC timestamp>` before recreating source metadata. This
avoids retaining stale relative-path registrations from an older project.
To regenerate:

```sh
source /path/to/Vitis/settings64.sh
cd Assignment_2/exercise7_neural_net/hls
vitis-run --mode hls --tcl run_hls.tcl
```
