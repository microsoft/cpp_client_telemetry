#!/bin/sh
apt install -y automake
apt install -y libtool-bin
apt install -y cmake
apt install -y sqlite
apt install -y curl libcurl4-openssl-dev
apt install -y zlib1g-dev
apt install -y git
apt install -y build-essential
apt install -y libssl-dev
apt install -y zlib1g-dev
apt install -y libsqlite3-dev
# Stock sqlite may be too old
#apt install libsqlite3-dev
apt install -y wget

# Install sqlite 3.22
export SQLITE_PKG=sqlite-autoconf-3220000
wget https://www.sqlite.org/2018/$SQLITE_PKG.tar.gz -O /tmp/sqlite-snapshot.tar.gz
tar -xvf /tmp/sqlite-snapshot.tar.gz
cd $SQLITE_PKG
./configure && make && make install
cd ..
