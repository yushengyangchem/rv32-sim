#!/usr/bin/env bash
set -euo pipefail

out_file="${1:-rtl/tb/demo_mem_init.memh}"

mkdir -p "$(dirname "$out_file")"

{
	echo "// Auto-generated demo memory image for GeMM / Reduction / SDPA"
	echo "// Word-addressed readmemh format with @<word_addr> jumps"
	echo
	echo "@00000400"
	echo "00000001"
	echo "00000002"
	echo "00000003"
	echo "00000004"
	echo "00000005"
	echo "00000006"
	echo
	echo "@00000800"
	echo "00000007"
	echo "00000008"
	echo "00000009"
	echo "0000000A"
	echo "0000000B"
	echo "0000000C"
	echo
	echo "@00000C00"
	echo "00000000"
	echo "00000000"
	echo "00000000"
	echo "00000000"
	echo
	echo "@00000C40"
	echo "00002000"
	echo "00003000"
	echo "00000002"
	echo "00000002"
	echo "00000003"
	echo
	echo "@00001000"
	echo "3F800000"
	echo "C0000000"
	echo "40600000"
	echo "40800000"
	echo "BFC00000"
	echo "40000000"
	echo
	echo "@00001040"
	echo "00000006"
	echo "00004200"
	echo
	echo "@00001080"
	echo "00000000"
	echo
	echo "@00001400"
	echo "3F800000"
	echo "00000000"
	echo "00000000"
	echo "3F800000"
	echo
	echo "@00001440"
	echo "00005200"
	echo "00005300"
	echo "00005400"
	echo "00000002"
	echo "00000002"
	echo "00000002"
	echo
	echo "@00001480"
	echo "3F800000"
	echo "00000000"
	echo "00000000"
	echo "3F800000"
	echo
	echo "@000014C0"
	echo "3F800000"
	echo "40000000"
	echo "40400000"
	echo "40800000"
	echo
	echo "@00001500"
	echo "00000000"
	echo "00000000"
	echo "00000000"
	echo "00000000"
} >"$out_file"

echo "[GEN] Wrote testbench memory image: $out_file"
