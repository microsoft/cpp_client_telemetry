#!/bin/sh
export PATH=/build/go/bin:$PATH
export GOROOT=/build/go
export GOPATH=`pwd`
go version
go build -o example
