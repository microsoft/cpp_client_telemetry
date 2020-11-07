#!/bin/bash
#
# TODO: this build requires Bazel. In order to build with CMake - see example here:
# https://github.com/ClickHouse/ClickHouse/pull/11590/files
#
export PATH=/usr/local/bin:$PATH
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd $DIR
cd ../third_party
git clone https://github.com/google/tcmalloc.git
cd tcmalloc
bazel test //tcmalloc/...
popd
