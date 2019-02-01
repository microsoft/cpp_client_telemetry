#!/bin/sh

if [ -f /bin/yum ]; then
# Prefer yum over apt
yum -y install automake
yum -y install autoconf
yum -y install libtool
yum -y install make gcc gcc-c++
yum -y install wget
yum -y install libcurl
yum -y install zlib-devel
yum -y install git
yum -y install gperftools-libs
yum -y install libcurl-devel
yum -y install rpm-build

# Install gcc-7
yum -y install centos-release-scl
yum -y install devtoolset-7
yum -y install devtoolset-7-valgrind

yum-config-manager --enable rhel-server-rhscl-7-rpms

if [ `gcc --version | grep 7` == "" ]; then
echo "*********************************************************"
echo "*** Please make sure you start the build with gcc-7   ***"
echo "*** > scl enable devtoolset-7 ./build.sh              ***"
echo "*********************************************************"
exit 3
fi

if [ `cmake --version | grep 3` == "" ]; then
yum -y remove cmake
wget https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz
tar -zxvf cmake-3.6.2.tar.gz
cd cmake-3.6.2
./bootstrap --prefix=/usr/local
make
make install
cd ..
fi

else
# Use apt
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
fi

## Install sqlite 3.22
export SQLITE_PKG=sqlite-autoconf-3220000
wget https://www.sqlite.org/2018/$SQLITE_PKG.tar.gz -O /tmp/sqlite-snapshot.tar.gz
tar -xvf /tmp/sqlite-snapshot.tar.gz
cd $SQLITE_PKG
./configure && make && make install
cd ..

## Build Google Test framework
./build-gtest.sh
