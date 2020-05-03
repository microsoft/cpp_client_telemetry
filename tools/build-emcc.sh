#!/bin/sh
# Template script that shows how to compile with Emscripten
# Usage:
# - naviate to root folder that contains CMakeLists.txt of a project you want to compile
# - launch 'build-emcc.sh'
# - observe the results under 'build' folder

mkdir -p build
cd build

DIR=`dirname $0`
echo DIR=$DIR

OS_NAME=`uname -a`
case "$OS_NAME" in
 *Darwin*) source $DIR/emcc-env-mac.sh  ;;
 *Linux*)  source $DIR/emcc-env-linux.sh ;;
 *)        echo "WARNING: unsupported OS $OS_NAME"
esac

emcmake cmake ..
emmake make
