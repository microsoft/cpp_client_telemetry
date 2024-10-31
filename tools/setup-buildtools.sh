#!/bin/sh

if [ -f /bin/yum ]; then
if [ `cat /etc/redhat-release | tr -dc '0-9.'|cut -d \. -f1` == "7" ]; then
# Prefer yum over apt-get
yum -y install automake
yum -y install autoconf
yum -y install libtool
yum -y install make gcc gcc-c++
yum -y install wget
yum -y install libcurl
yum -y install zlib-devel
yum -y install git
yum -y install gperftools-libs
yum -y install libcurl-devel nghttp2
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
echo "*** Nothing to install for that version CentOS  ***"
fi
else
# Use apt-get
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
apt-get install -y automake
apt-get install -y libtool-bin
apt-get install -y cmake
apt-get install -y sqlite
apt-get install -y curl
apt-get install -y libcurl4-openssl-dev nghttp2
apt-get install -y zlib1g-dev
apt-get install -y git
apt-get install -y build-essential
apt-get install -y libssl-dev
apt-get install -y libsqlite3-dev
# Stock sqlite may be too old
#apt install libsqlite3-dev
apt-get install -y wget
fi

## Install sqlite 3.44
export SQLITE_PKG=sqlite-autoconf-3440000
wget https://www.sqlite.org/2023/$SQLITE_PKG.tar.gz -O /tmp/sqlite-snapshot.tar.gz
tar -xvf /tmp/sqlite-snapshot.tar.gz
cd $SQLITE_PKG
./configure && make && make install
cd ..

## Build Google Test framework
./build-gtest.sh

## Change owner from root to current dir owner
chown -R `stat . -c %u:%g` *
