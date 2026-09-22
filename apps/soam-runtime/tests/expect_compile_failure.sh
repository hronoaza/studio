#!/usr/bin/env bash
set -euo pipefail

cxx="${CXX:-c++}"
src="$1"
include_runtime="$2"
include_domain="$3"
label="${4:-compile-fail case}"
include_extra="${5:-}"

include_args=(-I"$include_runtime" -I"$include_domain")
if [[ -n "$include_extra" ]]; then
  include_args+=(-I"$include_extra")
fi

if "$cxx" -std=c++20 -Wall -Wextra -Wpedantic -Werror     "${include_args[@]}"     -c "$src" -o /tmp/soam-expected-compile-failure.o; then
  echo "ERROR: $label unexpectedly compiled"
  exit 1
fi

echo "$label correctly rejected by compiler"
