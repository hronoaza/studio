#!/usr/bin/env bash
set -euo pipefail

cxx="${CXX:-c++}"
src="$1"
include_runtime="$2"
include_domain="$3"
label="${4:-compile-fail case}"

if "$cxx" -std=c++20 -Wall -Wextra -Wpedantic -Werror     -I"$include_runtime" -I"$include_domain"     -c "$src" -o /tmp/soam-expected-compile-failure.o; then
  echo "ERROR: $label unexpectedly compiled"
  exit 1
fi

echo "$label correctly rejected by compiler"
