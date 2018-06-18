#!/bin/bash

# Build first
./build.sh

# Start node.js test server
export PATH=.:./out:$PATH
export NODE_PATH=/usr/local/lib/node_modules
export PORT=18000

#nodejs --debug $NODE_PATH/http-echo-server/server.js &
./reflect.py &
SERVERPID=$!

# Wait for node.js to actually start the server
sleep 1

# Perform HTTP post
http-post

# Sleep a little bit to wait for result
sleep 1

kill -9 $SERVERPID
