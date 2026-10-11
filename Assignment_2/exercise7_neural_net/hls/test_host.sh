#!/usr/bin/env bash
set -euo pipefail

task_hls_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
task_include_dir=${1:-${XILINX_VITIS:-}/include}
if [[ ! -f "$task_include_dir/ap_int.h" ]]; then
    printf 'Usage: %s /path/to/Vitis/include [log-directory]\n' "$0" >&2
    exit 2
fi
task_log_dir=${2:-$(mktemp -d /tmp/erts-ex7-hls-tests.XXXXXX)}
mkdir -p -- "$task_log_dir"
task_compiler=${CXX:-g++}
task_sources=("$task_hls_dir/testbench.cpp" "$task_hls_dir/course_testbench.cpp"
              "$task_hls_dir/neuralNet.cpp")

"$task_compiler" -std=c++14 -I"$task_include_dir" "${task_sources[@]}" \
    -o "$task_log_dir/default_testbench"
"$task_log_dir/default_testbench" | tee "$task_log_dir/default.log"

"$task_compiler" -std=c++14 -I"$task_include_dir" \
    -DINPUT_LEN=8 -DNEURONS_LEN=3 -DOUTPUT_LEN=4 "${task_sources[@]}" \
    -o "$task_log_dir/alternate_testbench"
"$task_log_dir/alternate_testbench" | tee "$task_log_dir/alternate_8_3_4.log"

# The preserved original has its own original header: use the default dimensions.
"$task_compiler" -std=c++14 -I"$task_include_dir" \
    "$task_hls_dir/provided_course/testbench.cpp" "$task_hls_dir/neuralNet.cpp" \
    -o "$task_log_dir/provided_course_testbench"
set +e
"$task_log_dir/provided_course_testbench" | tee "$task_log_dir/provided_course.log"
task_original_status=${PIPESTATUS[0]}
set -e
if [[ "$task_original_status" -ne 1 ]]; then
    printf 'Unexpected original-course test exit: %s (expected mismatch exit 1).\n' \
        "$task_original_status" >&2
    exit 1
fi
printf 'Original course test remains FAIL (exit 1); known reference mismatch reproduced.\n'
printf 'Adapted course/custom tests PASS for default and 8/3/4 host configurations.\n'
printf 'Host test logs: %s\n' "$task_log_dir"
