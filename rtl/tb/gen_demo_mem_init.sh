#!/usr/bin/env bash
set -euo pipefail

out_file="${1:-rtl/tb/demo_mem_init.memh}"

mkdir -p "$(dirname "$out_file")"

{
	echo "// Auto-generated demo memory image for GeMM / Reduction / SDPA"
	echo "// Word-addressed readmemh format with @<word_addr> jumps"
	echo
	echo "@00020000"
	echo "00000001"
	echo "00000002"
	echo "00000003"
	echo "00000004"
	echo "00000005"
	echo "00000006"
	echo
	echo "@00020400"
	echo "00000007"
	echo "00000008"
	echo "00000009"
	echo "0000000A"
	echo "0000000B"
	echo "0000000C"
	echo
	echo "@00020800"
	echo "00000000"
	echo "00000000"
	echo "00000000"
	echo "00000000"
	echo
	echo "@00020840"
	echo "00081000"
	echo "00082000"
	echo "00000002"
	echo "00000002"
	echo "00000003"
	echo
	echo "@00020C00"
	echo "3F800000"
	echo "C0000000"
	echo "40600000"
	echo "40800000"
	echo "BFC00000"
	echo "40000000"
	echo
	echo "@00020C40"
	echo "00000006"
	echo "00083200"
	echo
	echo "@00020C80"
	echo "00000000"
	echo
	echo "@00021000"
	echo "3F800000"
	echo "00000000"
	echo "00000000"
	echo "3F800000"
	echo
	echo "@00021040"
	echo "00084200"
	echo "00084300"
	echo "00084400"
	echo "00000002"
	echo "00000002"
	echo "00000002"
	echo
	echo "@00021080"
	echo "3F800000"
	echo "00000000"
	echo "00000000"
	echo "3F800000"
	echo
	echo "@000210C0"
	echo "3F800000"
	echo "40000000"
	echo "40400000"
	echo "40800000"
	echo
	echo "@00021100"
	echo "00000000"
	echo "00000000"
	echo "00000000"
	echo "00000000"
} >"$out_file"

echo "[GEN] Wrote testbench memory image: $out_file"
