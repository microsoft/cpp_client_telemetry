#!/bin/sh

rm .buildtools
docker info
docker version

export IMAGE_NAME=$1
export BUILD_TYPE=$2
echo Running in container $IMAGE_NAME

echo Building docker image...
docker build --rm -t $IMAGE_NAME docker/$IMAGE_NAME

echo Starting build...
docker run -v `pwd`:/build -w /build $IMAGE_NAME ./build.sh $BUILD_TYPE
