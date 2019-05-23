@echo off
if "%~1"=="" goto help

del .buildtools 2>NUL
docker info
docker version

set IMAGE_NAME=%1
echo Running in container %IMAGE_NAME%
sc query com.docker.service

echo Building docker image...
docker build --rm -t %IMAGE_NAME% docker/%IMAGE_NAME%

echo Starting build...
docker run -it -v %CD%:/build %IMAGE_NAME% /build/build.sh

exit

:help
echo.
echo Usage: build-docker.cmd [container_name]
echo.
echo Supported containers:
dir /B docker
