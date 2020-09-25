#!/bin/sh
MATSDK_INSTALL_DIR=$1
echo "Install SDK to $MATSDK_INSTALL_DIR"
mkdir -p $MATSDK_INSTALL_DIR/lib
cp out/lib/libmat.* $MATSDK_INSTALL_DIR/lib
mkdir -p $MATSDK_INSTALL_DIR/include/mat
cp lib/include/public/* $MATSDK_INSTALL_DIR/include/mat
