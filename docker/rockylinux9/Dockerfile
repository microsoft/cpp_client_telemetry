FROM mipsdkcontainer.azurecr.io/rockylinux9

## Common packages for linux build environment
RUN yum update -y && \
yum install -y zlib-devel clang python pkg-config bzip2 unzip make wget sudo cmake && \
yum install -y zlib-static gmock gmock-devel gtest gtest-devel

CMD /bin/bash