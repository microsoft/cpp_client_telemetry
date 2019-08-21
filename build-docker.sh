#!/bin/sh

rm .buildtools
docker info
docker version

export IMAGE_NAME=$1
echo Running in container $IMAGE_NAME

echo Building docker image...
docker build --rm -t $IMAGE_NAME docker/$IMAGE_NAME

echo Starting build...
docker run -i -v `pwd`:/build $IMAGE_NAME /build/build.sh
