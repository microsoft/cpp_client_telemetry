#!/bin/bash

#
# Clean-up previous results
#
sudo rm /tmp/aria*.log
sudo rm offline*
sudo rm -f heap*

BIN=./example

case $1 in

"")
  echo "Running simple test..."
  $BIN
  ;;

1)
  echo "Running heap check..."
  export PPROF_PATH=/usr/local/bin
  export HEAPCHECK=normal
# export HEAPPROFILE=/tmp/heapprof
# export LD_PRELOAD="/usr/lib/libtcmalloc.so"
  $BIN
  ;;

2)
  echo "Running gdb..."
  gdb -ex=r --args $BIN
  ;;

3)
  echo "Running valgrind..."
  valgrind -v $BIN
  ;;

4)
  echo "Running valgrind leak check..."
  valgrind -v --track-origins=yes --leak-check=full $BIN
# valgrind --vgdb=yes --vgdb-error=0 ./example
  ;;

5)
  echo "Running cgroups 500MB memory test..."
  cgcreate -g memory:/500MB
  echo $(( 500 * 1024 * 1024 )) > /sys/fs/cgroup/memory/500MB/memory.limit_in_bytes
  cgexec -g memory:/500MB $BIN
  ;;

esac
