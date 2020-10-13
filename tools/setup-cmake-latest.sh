#!/bin/bash
#
# This script installs latest CMake on Linux machine
#
export PATH=/usr/local/bin:$PATH

if [[ ! -f "/tmp/cmake.tar.gz" ]]; then
  wget -O /tmp/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4.tar.gz
  tar -zxvf /tmp/cmake.tar.gz
fi

# Bootstrap CMake
cd cmake-3.18.4
./bootstrap --prefix=/usr/local

# Build CMake with ninja
mkdir out
cd out
cmake .. -G ninja
if [ ! command -v COMMAND &> /dev/null ]; then
  # nina must be installed!
  wget -O /tmp/ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.10.1/ninja-linux.zip
  sudo unzip /tmp/ninja.zip -d /usr/local/bin/
fi
ninja
ninja install
