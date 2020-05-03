#!/bin/sh

export WORKSPACE=`pwd`
export PATH=`pwd`/tools:$PATH

export SYSROOT=`pwd`/out/emscripten
if [ ! -d $SYSROOT ];
then
  mkdir -p $SYSROOT
fi

cd $SYSROOT
git clone https://github.com/emscripten-ports/zlib
cd zlib
build-emcc.sh

cd $WORKSPACE/tools/ddv.js/decoder
rm -rf build
build-emcc.sh
