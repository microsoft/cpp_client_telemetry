FROM mipsdkcontainer.azurecr.io/rockylinux9

## Common packages for linux build environment
RUN yum install -y yum-utils && \
yum-config-manager --enable crb && \
yum update -y && \
yum install -y zlib-devel clang python pkg-config bzip2 unzip make wget sudo cmake && \
yum -y install epel-release && \
yum install -y zlib-static gmock gmock-devel gtest gtest-devel

CMD /bin/bash