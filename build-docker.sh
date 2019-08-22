#!/bin/sh

rm .buildtools
docker info
docker version

export IMAGE_NAME=$1
echo Running in container $IMAGE_NAME

echo Building docker image...
docker build --rm -t $IMAGE_NAME docker/$IMAGE_NAME

echo Starting build...
docker run -v `pwd`:/mnt/s -w /mnt/s $IMAGE_NAME build.sh
