FROM mipsdkcontainer.azurecr.io/library/centos:7

# Install Dependencies

USER 0
RUN \
  yum install -y epel-release && \
  yum -y update && \
  yum -y install libxml2-devel libcurl-devel wget gdk-pixbuf2 glib2-devel python2-scons libuuid-devel && \
  yum -y install libsecret libsecret-devel gnome-keyring

RUN \
  yum -y install centos-release-scl && \
  yum-config-manager --enable rhel-server-rhscl-7-rpms && \
  yum -y install devtoolset-7 && \
  scl enable devtoolset-7 bash

ENV MANPATH /opt/rh/devtoolset-7/root/usr/share/man:
ENV PERL5LIB /opt/rh/devtoolset-7/root//usr/lib64/perl5/vendor_perl:/opt/rh/devtoolset-7/root/usr/lib/perl5:/opt/rh/devtoolset-7/root//usr/share/perl5/vendor_perl
ENV X_SCLS devtoolset-7
ENV PCP_DIR /opt/rh/devtoolset-7/root
ENV LD_LIBRARY_PATH /opt/rh/devtoolset-7/root/usr/lib64:/opt/rh/devtoolset-7/root/usr/lib:/opt/rh/devtoolset-7/root/usr/lib64/dyninst:/opt/rh/devtoolset-7/root/usr/lib/dyninst:/opt/rh/devtoolset-7/root/usr/lib64:/opt/rh/devtoolset-7/root/usr/lib
ENV PATH /opt/rh/devtoolset-7/root/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
ENV INFOPATH /opt/rh/devtoolset-7/root/usr/share/info

## Common packages for linux build environment
RUN yum install -y gcc gcc-c++ automake libtool-bin curl libcurl4-openssl-dev zlib1g-dev zlib-static build-essential libssl-dev clang python pkg-config git curl bzip2 unzip make wget sudo centos-release-scl devtoolset-7 devtoolset-7-valgrind

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
