#!/bin/bash

## Uncomment for custom path to golang
#export PATH=/build/go/bin:$PATH
#export GOROOT=/build/go
export GOPATH=`pwd`

# Determine OS
OS_NAME=`uname -a`
case "$OS_NAME" in
 *Darwin*) OS=mac ;;
 *Linux*)  OS=linux ;;
 *)        echo "WARNING: unsupported OS $OS_NAME , exiting.."
           return 0 ;;
esac

# Link to proper build file for OS
MODULE_PATH=`pwd`/src/Microsoft/Telemetry/EventLogger
unlink $MODULE_PATH/build.go
ln -s $MODULE_PATH/build.go.$OS $MODULE_PATH/build.go

go version
go build -o example
