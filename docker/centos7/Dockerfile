FROM centos/devtoolset-7-toolchain-centos7

USER 0

# Package installation
RUN yum update -y

## Common packages for linux build environment
RUN yum install -y gcc gcc-c++ automake libtool-bin curl nghttp2 libcurl4-openssl-dev zlib1g-dev zlib-static build-essential libssl-dev clang python pkg-config git curl bzip2 unzip make wget sudo centos-release-scl devtoolset-7 devtoolset-7-valgrind

## Add docker user
RUN useradd -m docker
RUN passwd -d docker
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN echo 'docker ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
RUN mkdir -p /tmp
## RUN mount -t tmpfs -o size=512M tmpfs /tmp
## USER docker

RUN echo "scl enable devtoolset-7 bash" >> ~/.bash_profile
RUN sudo yum update -y

## Install cmake3
RUN sudo bash -c 'wget -O /tmp/cmake-3.6.2.tar.gz https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz && \
	cd /tmp && \
	tar -zxvf cmake-3.6.2.tar.gz && \
	cd cmake-3.6.2 && \
	./bootstrap --prefix=/usr/local && \
	make && \
	make install'

CMD /bin/bash

# ENTRYPOINT bash
