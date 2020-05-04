#!/bin/sh

export WORKSPACE=`pwd`
export PATH=$WORKSPACE/tools:$PATH
export SYSROOT=$WORKSPACE/out/emscripten

if [ ! -d $SYSROOT ];
then
  mkdir -p $SYSROOT
fi

cd $SYSROOT
git clone https://github.com/emscripten-ports/zlib
cd zlib
build-emcc.sh

cd $WORKSPACE/tools/ddv.js/decoder
# Always perform clean build
rm -rf build
build-emcc.sh
