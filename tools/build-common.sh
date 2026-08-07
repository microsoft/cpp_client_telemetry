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
  local input="${CMAKE_OPTS:-}"
  local token=""
  local char=""
  local escaped=false
  local in_single_quote=false
  local in_double_quote=false
  local token_started=false
  local index
  local length=${#input}
  local -a parsed_args=()

  # Parse the existing shell-like CMAKE_OPTS format without evaluating it.
  for ((index = 0; index < length; index++)); do
    char="${input:index:1}"
    if [[ "$escaped" == true ]]; then
      token+="$char"
      escaped=false
      token_started=true
    elif [[ "$char" == "\\" && "$in_single_quote" == false ]]; then
      escaped=true
      token_started=true
    elif [[ "$char" == "'" && "$in_double_quote" == false ]]; then
      if [[ "$in_single_quote" == true ]]; then
        in_single_quote=false
      else
        in_single_quote=true
      fi
      token_started=true
    elif [[ "$char" == '"' && "$in_single_quote" == false ]]; then
      if [[ "$in_double_quote" == true ]]; then
        in_double_quote=false
      else
        in_double_quote=true
      fi
      token_started=true
    elif [[ "$char" =~ [[:space:]] && "$in_single_quote" == false && "$in_double_quote" == false ]]; then
      if [[ "$token_started" == true ]]; then
        parsed_args+=("$token")
        token=""
        token_started=false
      fi
    else
      token+="$char"
      token_started=true
    fi
  done

  if [[ "$escaped" == true || "$in_single_quote" == true || "$in_double_quote" == true ]]; then
    echo "Error: CMAKE_OPTS contains an unterminated escape or quote." >&2
    return 1
  fi
  if [[ "$token_started" == true ]]; then
    parsed_args+=("$token")
  fi

  cmake_args+=("${parsed_args[@]}")
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
