#!/bin/sh

# Run these commands to install gcc-5.x compiler on Debian 8.x container.
# 1DS C/C++ SDK requires gcc-5.x or newer compiler.
echo "deb http://ftp.us.debian.org/debian unstable main contrib non-free" >> /etc/apt/sources.list.d/unstable.list
apt-get update
apt-get install -y -t unstable gcc-5
apt-get install -y zlib1g-dev libsqlite3-dev cmake
apt-get install -y git
apt-get install -y g++-5
apt-get install -y build-essential
apt-get install -y libcurl4-openssl-dev
apt-get install -y libssl-dev
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 60 --slave /usr/bin/g++ g++ /usr/bin/g++-5
# After gcc-5.x is deployed, you may remove 'unstable' deps
