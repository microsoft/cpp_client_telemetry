# Linux build tools

## Required dependencies

- gcc-5+ or above
- zlib
- sqlite3
- libcurl + openssl
- gtest (optional)

### Installing dependencies as root

```console
sudo apt-get install cmake3 sqlite3 libsqlite3-dev libcurl4-openssl-dev libgtest-dev
```

There is a helper script in source code distribution that can be invoked as follows:

```console
source tools/setup-buildtools.sh
```

### Debian 8.x Jesse (old stable) specific instructions

1DS SDK requires gcc-5+. Latest available in Debian 8 and below is 4.9. gcc-5 and g++-5 must to be installed manually:

```console
#!/bin/sh
# Run these commands to install gcc-5.x compiler on Debian 8.x container.
# Microsoft Events SDK requires gcc-5.x or newer compiler.
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
```

### Installing gcc-5.x on older Linux distributions (Ubuntu 14.04, Debian 8.x and below)

This step is not required for recent distros which come with gcc version above 5+.
Microsoft Events SDK expects gcc-5.x+ which is not included in Ubuntu-14.04, Debian 8 and below.
Please install  recent gcc-5.x on older Ubuntu distributions as follows:

```console
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-5 g++-5
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 60 --slave /usr/bin/g++ g++ /usr/bin/g++-5
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 1
# Check that gcc is the proper 5.x version
gcc --version
# Output:
gcc (Ubuntu 5.4.1-2ubuntu1~14.04) 5.4.1 20160904
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Checkout and Build Microsoft Events OneSDK from source

Please contact OneSDK dev team for the git repository location. Building Microsoft Events SDK from source:

```console
cmake .
make
```

Package for your platform is going to be created and placed in ./out directory.
