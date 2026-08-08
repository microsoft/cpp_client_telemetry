#!/bin/sh
set -e

MATSDK_INSTALL_DIR=${1:-/usr/local}
if [ ! -f out/cmake_install.cmake ]; then
  echo "ERROR: out/cmake_install.cmake not found; configure and build the SDK first." >&2
  exit 1
fi

echo "Install SDK to $MATSDK_INSTALL_DIR"
cmake --install out --prefix "$MATSDK_INSTALL_DIR"
