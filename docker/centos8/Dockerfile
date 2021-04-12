FROM centos:centos8

USER 0

# Package installation
RUN yum update -y

## Common packages for linux build environment
RUN yum -y group install "Development Tools"
RUN yum install -y curl libcurl-devel zlib-devel clang python38 pkg-config git bzip2 unzip make wget sudo cmake
RUN yum --enablerepo=powertools install -y zlib-static gmock gtest

CMD /bin/bash