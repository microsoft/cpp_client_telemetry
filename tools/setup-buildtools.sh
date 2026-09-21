#!/bin/bash

CMAKE_VERSION=4.4.2

install_cmake()
{
if command -v cmake >/dev/null 2>&1 && \
   [ "$(printf '%s\n' 4.3 "$(cmake --version | head -1 | awk '{print $3}')" | sort -V | head -1)" = "4.3" ]; then
return
fi

case "$(uname -m)" in
x86_64)
  CMAKE_ARCH=x86_64
  CMAKE_SHA256=3ada9a3f5d8a85413579bdd0ea6aa8e8da86efdd6d15c91a1afa517f2021956c
  ;;
aarch64|arm64)
  CMAKE_ARCH=aarch64
  CMAKE_SHA256=9ca1aadb4451c5dcbdc67f9b4aff42dab52abbaebd8db9e2900026502dbed671
  ;;
*)
  echo "No prebuilt CMake is configured for $(uname -m)." >&2
  echo "Install CMake 4.3 or newer and rerun the build." >&2
  exit 1
  ;;
esac

CMAKE_ARCHIVE="cmake-${CMAKE_VERSION}-linux-${CMAKE_ARCH}.tar.gz"
CMAKE_URL="https://cmake.org/files/v4.4/${CMAKE_ARCHIVE}"
CMAKE_INSTALL_DIR="/opt/cmake-${CMAKE_VERSION}"
wget -q "${CMAKE_URL}" -O "/tmp/${CMAKE_ARCHIVE}" || return 1
echo "${CMAKE_SHA256}  /tmp/${CMAKE_ARCHIVE}" | sha256sum --check - || return 1
mkdir -p "${CMAKE_INSTALL_DIR}" || return 1
tar -xzf "/tmp/${CMAKE_ARCHIVE}" \
  --strip-components=1 -C "${CMAKE_INSTALL_DIR}" || return 1
rm -f "/tmp/${CMAKE_ARCHIVE}" || return 1
for tool in cmake cpack ctest; do
  ln -sf "${CMAKE_INSTALL_DIR}/bin/${tool}" "/usr/local/bin/${tool}" || return 1
done
}

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

else
echo "*** Nothing to install for that version CentOS  ***"
fi
else
# Use apt-get
export DEBIAN_FRONTEND=noninteractive
apt-get update -y
apt-get install -y automake
apt-get install -y libtool-bin
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

install_cmake || exit 1

## Install sqlite 3.22
export SQLITE_PKG=sqlite-autoconf-3220000
wget https://www.sqlite.org/2018/$SQLITE_PKG.tar.gz -O /tmp/sqlite-snapshot.tar.gz
tar -xvf /tmp/sqlite-snapshot.tar.gz
cd $SQLITE_PKG
./configure && make && make install
cd ..

## Change owner from root to current dir owner
chown -R `stat . -c %u:%g` *
