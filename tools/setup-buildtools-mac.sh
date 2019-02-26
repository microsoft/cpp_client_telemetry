#!/bin/sh
brew install cmake
brew install wget

## Install sqlite 3.22
export SQLITE_PKG=sqlite-autoconf-3220000
wget https://www.sqlite.org/2018/$SQLITE_PKG.tar.gz -O /tmp/sqlite-snapshot.tar.gz
tar -xvf /tmp/sqlite-snapshot.tar.gz
cd $SQLITE_PKG
./configure && make && make install
cd ..

## Build Google Test framework
./build-gtest.sh
