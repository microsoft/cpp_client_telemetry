#!/usr/bin/env bash
#
# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Public header gate (GCC/Clang).
#
# Verifies that every public SDK header is self-contained (compiles on its own,
# in any include order) and warning-clean under strict, consumer-representative
# warning flags. Downstream consumers such as ONNX Runtime / Foundry Local
# compile their own translation units -- which include these headers -- with
# -Wall -Wextra -Werror (plus -Wshorten-64-to-32 on Clang). This gate compiles
# each public header on its own, with no -isystem suppression, so any header
# issue surfaces here instead of at integration time.
#
# Exits non-zero if any header fails to compile or emits a warning.

set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PUB="$REPO_ROOT/lib/include/public"

# Implementation-fragment headers: intentionally included by another public
# header (which supplies their dependencies first) and not meant to be included
# standalone. They are exercised through their public entry point instead.
EXCLUDES=(
  "VariantType.hpp"   # included by Variant.hpp, which defines VariantMap/VariantArray first
)

INCLUDES="-I$PUB -I$REPO_ROOT/lib/include"
# Mirrors this repo's own warning flags and what ORT applies to its C++ TUs.
BASE_FLAGS="-std=c++17 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-but-set-variable"

is_excluded() {
  local n="$1" e
  for e in "${EXCLUDES[@]}"; do [ "$e" = "$n" ] && return 0; done
  return 1
}

fail=0
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

run_compiler() {
  local cc="$1" extra="$2" label="$3"
  local n_ok=0 n_fail=0 h name tu out
  echo "== $label =="
  for h in "$PUB"/*.hpp; do
    name="$(basename "$h")"
    is_excluded "$name" && continue
    tu="$tmp/tu_${name%.hpp}.cpp"
    printf '#include "%s"\nint main() { return 0; }\n' "$name" > "$tu"
    if out="$("$cc" $BASE_FLAGS $extra $INCLUDES -fsyntax-only "$tu" 2>&1)"; then
      n_ok=$((n_ok + 1))
    else
      n_fail=$((n_fail + 1)); fail=1
      echo "  FAIL: $name"
      echo "$out" | grep -E 'error:|warning:' | head -4 | sed 's/^/      /'
    fi
  done
  echo "  $label: $n_ok passed, $n_fail failed"
}

ran=0
if command -v g++ >/dev/null 2>&1; then
  run_compiler g++ "" "g++  (c++17, -Wall -Wextra -Werror)"
  ran=1
fi
if command -v clang++ >/dev/null 2>&1; then
  run_compiler clang++ "-Wshorten-64-to-32" "clang++ (c++17, + -Wshorten-64-to-32)"
  ran=1
fi

if [ "$ran" -eq 0 ]; then
  echo "error: neither g++ nor clang++ was found" >&2
  exit 2
fi

if [ "$fail" -ne 0 ]; then
  echo "Public header gate FAILED."
  exit 1
fi
echo "Public header gate passed."
