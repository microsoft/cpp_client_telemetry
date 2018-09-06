#!/bin/bash

export PATH=/usr/local/bin:$PATH

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Current directory: $DIR"
cd $DIR

if [ "$1" == "clean" ]; then
 rm -f CMakeCache.txt *.cmake
 rm -rf out
 rm -rf .buildtools
# make clean
fi

# Install build tools and recent sqlite3
FILE=.buildtools
if [ ! -f .buildtools ]; then
  sudo tools/setup-buildtools.sh
  echo >.buildtools
fi

if [ -f /usr/bin/gcc ]; then
echo "gcc version: `gcc --version`"
fi

#rm -rf out
mkdir -p out
cd out

if [ -f /usr/bin/rpmbuild ]; then
export CMAKE_PACKAGE_TYPE=rpm
else
export CMAKE_PACKAGE_TYPE=deb
fi

if [ "$2" == "release" ]; then
# TODO: pass custom build flags?
  cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE ..
# TODO: strip symbols to minimize
else
  cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PACKAGE_TYPE=$CMAKE_PACKAGE_TYPE ..
fi

# Build all
make

# Remove old package
rm -f *.deb *.rpm

# Build new package
make package

if [ -f /usr/bin/dpkg ]; then
# Install new package
sudo dpkg -i *.deb
fi

# TODO: remove this section below ... consider moving 'strip' commands to release configuration above
#
# Install SDK headers and lib to /usr/local
#
#cd ..
#ARIA_SDK_INSTALL_DIR="${ARIA_SDK_INSTALL_DIR:-/usr/local}"
#echo "Install SDK to $ARIA_SDK_INSTALL_DIR .."
#sudo mkdir -p $ARIA_SDK_INSTALL_DIR/lib/aria
#sudo cp out/lib/libaria.a $ARIA_SDK_INSTALL_DIR/lib/aria
## strip --strip-unneeded out/lib/libaria.so
## strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/lib/libaria.so
#sudo cp out/lib/libaria.so $ARIA_SDK_INSTALL_DIR/lib/aria
#sudo mkdir -p $ARIA_SDK_INSTALL_DIR/include/aria
#sudo cp lib/include/public/* $ARIA_SDK_INSTALL_DIR/include/aria
