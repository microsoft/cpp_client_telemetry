#!/bin/sh

# TODO: it's not ideal experience, but we use have to use brew-provided deps.
# Sometimes we might run into a situation where a different user takes over
# control of the brew dirs. That causes the brew update to fail.
# Temporarily allow the user to take over control of brew files.

if [ -z "$NOROOT" ]; then
  echo "***"
  echo "*** You may need to enter your admin password to update the brew files:"
  echo "***"
fi

ensure_ownership () {
  for PATH_TO_CHECK in "$@"; do
    if [ ! -e "$PATH_TO_CHECK" ]; then
      echo "ensure_ownership: $PATH_TO_CHECK doesn't exist, Homebrew may not be installed properly"
      continue
    fi

    if [ -O "$PATH_TO_CHECK" ]; then
      continue
    fi

    if [ -z "$NOROOT" ]; then
      sudo chown -R $(whoami) "$PATH_TO_CHECK"
      continue
    fi

    echo "ensure_ownership (NOROOT): Path $PATH_TO_CHECK is not owned by current user, Homebrew may not work properly and fail the build"
    echo "ensure_ownership (NOROOT): Fix the ownership of required paths or run the build without NOROOT flag"
  done
}

ensure_ownership \
  /usr/local/Cellar \
  /usr/local/Homebrew \
  /usr/local/var/homebrew \
  /usr/local/etc/bash_completion.d \
  /usr/local/include \
  /usr/local/lib/pkgconfig \
  /usr/local/share/aclocal \
  /usr/local/share/locale \
  /usr/local/share/zsh \
  /usr/local/share/zsh/site-functions \
  /usr/local/var/homebrew/locks

brew install cmake
brew install wget

## Install sqlite 3.22
export SQLITE_PKG=sqlite-autoconf-3390000
wget https://www.sqlite.org/2022/$SQLITE_PKG.tar.gz -O /tmp/sqlite-snapshot.tar.gz
tar -xvf /tmp/sqlite-snapshot.tar.gz
cd $SQLITE_PKG
./configure && make && make install
cd ..

## Build Google Test framework
./build-gtest.sh $1

## Install dotnet for test server
