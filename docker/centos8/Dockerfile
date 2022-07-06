FROM mipsdkcontainer.azurecr.io/centos8

USER 0


RUN sed -i 's/mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-Linux-* &&\
    sed -i 's|#baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|g' /etc/yum.repos.d/CentOS-Linux-*

# Package installation
RUN yum update -y

## Common packages for linux build environment
RUN yum -y group install "Development Tools"
RUN yum install -y curl libcurl-devel nghttp2 zlib-devel clang python38 pkg-config git bzip2 unzip make wget sudo cmake
RUN yum --enablerepo=powertools install -y zlib-static gmock gtest

CMD /bin/bash