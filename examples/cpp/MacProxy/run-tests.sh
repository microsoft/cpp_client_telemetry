#!/bin/bash

##
## Clean-up previous results
##
#sudo rm /tmp/aria*.log
#sudo rm offline*
#sudo rm -f heap*

BIN=./out/MacProxy

case $1 in

"")
  echo "Running basic test..."
  $BIN
  ;;

1)
  echo "Running heap check..."
  export PPROF_PATH=/usr/local/bin
  export HEAPCHECK=normal
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
  ;;

5)
  echo "Running cgroups 300MB memory test..."
  cgcreate -g memory:/300MB
  echo $(( 300 * 1024 * 1024 )) > /sys/fs/cgroup/memory/300MB/memory.limit_in_bytes
  cgexec -g memory:/300MB $BIN
  ;;

esac
