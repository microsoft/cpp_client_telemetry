FROM mipsdkcontainer.azurecr.io/rockylinux9

## Common packages for linux build environment
RUN yum install -y yum-utils && \
yum-config-manager --enable crb && \
yum update -y && \
yum -y group install "Development Tools" && \
yum install -y --setopt=tsflags=nodocs libcurl-devel zlib-devel clang python pkg-config git bzip2 unzip make wget sudo cmake && \
yum -y install epel-release --setopt=tsflags=nodocs && \
yum install -y --setopt=tsflags=nodocs zlib-static gmock gmock-devel gtest gtest-devel

CMD /bin/bash