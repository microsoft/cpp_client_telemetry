FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

# Package installation
RUN apt-get update

## Common packages for linux build environment
RUN apt install -y clang python pkg-config git curl bzip2 unzip make wget cmake sudo

RUN adduser --disabled-password --gecos '' docker
RUN adduser docker sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
## RUN useradd -m docker && echo "docker:docker" | chpasswd && adduser docker sudo
## USER docker

# this is where I was running into problems with the other approaches
RUN sudo apt-get update

CMD /bin/bash

# ENTRYPOINT bash
