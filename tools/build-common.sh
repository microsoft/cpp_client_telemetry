#!/bin/bash

matsdk_clean_build_outputs() {
  local script_name="$1"

  echo "$script_name: cleaning previous build artifacts"
  rm -f CMakeCache.txt *.cmake
  rm -rf out
  rm -rf .buildtools
}

matsdk_mark_buildtools_checked() {
  local marker_file="$1"

  echo > "$marker_file"
}

matsdk_install_buildtools_once() {
  local marker_file="$1"
  shift

  if [ ! -f "$marker_file" ]; then
    if [ $# -gt 0 ]; then
      "$@"
    fi
    matsdk_mark_buildtools_checked "$marker_file"
  fi
}

matsdk_try_buildtools_once() {
  local marker_file="$1"
  local failure_message="$2"
  shift 2

  if [ ! -f "$marker_file" ]; then
    if [ $# -gt 0 ]; then
      "$@" || echo "$failure_message"
    fi
    matsdk_mark_buildtools_checked "$marker_file"
  fi
}

matsdk_print_compiler_versions() {
  if [ -f /usr/bin/gcc ]; then
    echo "gcc   version: `gcc --version`"
  fi

  if [ -f /usr/bin/clang ]; then
    echo "clang version: `clang --version`"
  fi
}

matsdk_require_cmake_preset_support() {
  cmake -P "$DIR/cmake/MatsdkRequirePresetSupport.cmake"
}

matsdk_append_cmake_opts_to_cmake_args() {
  if [[ -n "${CMAKE_OPTS:-}" ]]; then
    # Preserve existing support for callers passing multiple quoted -D arguments.
    eval "extra_cmake_args=($CMAKE_OPTS)"
    cmake_args+=("${extra_cmake_args[@]}")
  fi
}

matsdk_run_logged_command() {
  printf ' %q' "$@"
  printf '\n'
  "$@"
}

matsdk_build_and_package_preset() {
  local preset="$1"

  cmake --build --preset "$preset"
  cmake --build --preset "$preset" --target package
}
