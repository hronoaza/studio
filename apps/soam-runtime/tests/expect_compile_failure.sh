#!/usr/bin/env bash
set -euo pipefail

cxx="${CXX:-c++}"
src="$1"
include_runtime="$2"
include_domain="$3"

if "$cxx" -std=c++20 -Wall -Wextra -Wpedantic -Werror     -I"$include_runtime" -I"$include_domain"     -c "$src" -o /tmp/d7-provenance-gate.o; then
  echo "ERROR: D7 provenance-injection misuse unexpectedly compiled"
  exit 1
fi

echo "D7 provenance-injection misuse correctly rejected by compiler"
