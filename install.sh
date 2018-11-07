#!/bin/sh
ARIA_SDK_INSTALL_DIR=$1
echo "Install SDK to $ARIA_SDK_INSTALL_DIR"
mkdir -p $ARIA_SDK_INSTALL_DIR/lib/aria
cp out/lib/libaria.a $ARIA_SDK_INSTALL_DIR/lib
mkdir -p $ARIA_SDK_INSTALL_DIR/include/aria
cp lib/include/public/* $ARIA_SDK_INSTALL_DIR/include/aria
