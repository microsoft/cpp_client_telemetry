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
# each public header on its own as both C++11 and C++17, with no -isystem
# suppression, so any header issue surfaces here instead of at integration time.
# It also compiles the ABI-stable C API header (mat.h) as C11.
#
# Exits non-zero if any header fails to compile or emits a warning.

set -uo pipefail
shopt -s nullglob

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PUB="$REPO_ROOT/lib/include/public"

# Fail fast if the public header directory is missing or miscomputed. With
# nullglob on, a bad PUB would make the header globs below expand to nothing and
# the gate would silently "pass" without compiling anything -- a false negative.
if [ ! -d "$PUB" ]; then
  echo "error: public header directory not found: $PUB" >&2
  exit 2
fi

# Implementation-fragment headers: intentionally included by another public
# header (which supplies their dependencies first) and not meant to be included
# standalone. They are exercised through their public entry point instead.
EXCLUDES=(
  "VariantType.hpp"   # included by Variant.hpp, which defines VariantMap/VariantArray first
)

# Mirror the strict warning flags third-party consumers build with
# (-Wall -Wextra -Werror, matching this repo's pipeline and ORT). Unused
# parameters are deliberately NOT suppressed: intentional ones use the
# UNREFERENCED_PARAMETER macro, which expands to (void)(...) on GCC/Clang, so the
# headers stay warning-clean without a blanket -Wno-unused-parameter that would
# hide a real consumer break (e.g. an override that leaves a parameter unused).
C_API_HEADER="mat.h"

is_excluded() {
  local n="$1" e
  for e in "${EXCLUDES[@]}"; do [ "$e" = "$n" ] && return 0; done
  return 1
}

fail=0
tmp=""
for _ in 1 2 3 4 5; do
  candidate="$REPO_ROOT/.public-header-gate.$$.$RANDOM"
  if mkdir "$candidate" 2>/dev/null; then
    tmp="$candidate"
    break
  fi
done
if [ -z "$tmp" ]; then
  echo "error: failed to create work directory under $REPO_ROOT" >&2
  exit 2
fi
trap 'rm -rf "$tmp"' EXIT

run_cxx_compiler() {
  local cc="$1" std="$2" label="$3"
  shift 3
  local n_ok=0 n_fail=0 h name base tu out
  echo "== $label =="
  for h in "$PUB"/*.hpp "$PUB"/*.h; do
    name="$(basename "$h")"
    is_excluded "$name" && continue
    base="${name%.*}"
    tu="$tmp/tu_${base}_${std}.cpp"
    printf '#include "%s"\nint main() { return 0; }\n' "$name" > "$tu"
    if out="$("$cc" "-std=$std" -Wall -Wextra -Werror "$@" -I"$PUB" -I"$REPO_ROOT/lib/include" -fsyntax-only "$tu" 2>&1)"; then
      n_ok=$((n_ok + 1))
    else
      n_fail=$((n_fail + 1)); fail=1
      echo "  FAIL: $name"
      echo "$out" | grep -E 'error:|warning:' | head -4 | sed 's/^/      /'
    fi
  done
  if [ $((n_ok + n_fail)) -eq 0 ]; then
    echo "  ERROR: no public headers found under $PUB"
    fail=1
  fi
  echo "  $label: $n_ok passed, $n_fail failed"
}

run_c_compiler() {
  local cc="$1" label="$2"
  shift 2
  local tu out
  echo "== $label =="
  if [ ! -f "$PUB/$C_API_HEADER" ]; then
    echo "  ERROR: C API header not found: $PUB/$C_API_HEADER"
    fail=1
    echo "  $label: 0 passed, 1 failed"
    return
  fi
  tu="$tmp/tu_${C_API_HEADER%.h}_c11.c"
  printf '#include "%s"\nint main(void) { return 0; }\n' "$C_API_HEADER" > "$tu"
  if out="$("$cc" -std=c11 -Wall -Wextra -Werror "$@" -I"$PUB" -I"$REPO_ROOT/lib/include" -fsyntax-only "$tu" 2>&1)"; then
    echo "  $label: 1 passed, 0 failed"
  else
    fail=1
    echo "  FAIL: $C_API_HEADER"
    echo "$out" | grep -E 'error:|warning:' | head -4 | sed 's/^/      /'
    echo "  $label: 0 passed, 1 failed"
  fi
}

cxx_ran=0
if command -v g++ >/dev/null 2>&1; then
  run_cxx_compiler g++ c++11 "g++  (c++11, -Wall -Wextra -Werror)"
  run_cxx_compiler g++ c++17 "g++  (c++17, -Wall -Wextra -Werror)"
  cxx_ran=1
fi
if command -v clang++ >/dev/null 2>&1; then
  run_cxx_compiler clang++ c++11 "clang++ (c++11, + -Wshorten-64-to-32)" -Wshorten-64-to-32
  run_cxx_compiler clang++ c++17 "clang++ (c++17, + -Wshorten-64-to-32)" -Wshorten-64-to-32
  cxx_ran=1
fi

if [ "$cxx_ran" -eq 0 ]; then
  echo "error: neither g++ nor clang++ was found" >&2
  exit 2
fi

c_ran=0
if command -v gcc >/dev/null 2>&1; then
  run_c_compiler gcc "gcc  (c11 mat.h, -Wall -Wextra -Werror)"
  c_ran=1
fi
if command -v clang >/dev/null 2>&1; then
  run_c_compiler clang "clang (c11 mat.h, + -Wshorten-64-to-32)" -Wshorten-64-to-32
  c_ran=1
fi
if [ "$c_ran" -eq 0 ] && command -v cc >/dev/null 2>&1; then
  run_c_compiler cc "cc   (c11 mat.h, -Wall -Wextra -Werror)"
  c_ran=1
fi

if [ "$c_ran" -eq 0 ]; then
  echo "error: no C compiler (gcc, clang, or cc) was found" >&2
  exit 2
fi

if [ "$fail" -ne 0 ]; then
  echo "Public header gate FAILED."
  exit 1
fi
echo "Public header gate passed."
