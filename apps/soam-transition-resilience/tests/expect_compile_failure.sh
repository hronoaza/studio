#!/usr/bin/env bash
set -euo pipefail
cxx="${CXX:-c++}"
src="$1"
label="$2"
shift 2
include_args=()
for include_dir in "$@"; do include_args+=(-I"$include_dir"); done
if "$cxx" -std=c++20 -Wall -Wextra -Wpedantic -Werror     "${include_args[@]}" -c "$src" -o /tmp/soam-resilience-expected-failure.o; then
  echo "ERROR: $label unexpectedly compiled"
  exit 1
fi
echo "$label correctly rejected by compiler"
