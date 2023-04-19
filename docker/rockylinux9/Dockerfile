FROM mipsdkcontainer.azurecr.io/rockylinux9

## Common packages for linux build environment
RUN yum install -y yum-utils && \
yum-config-manager --enable crb && \
yum update -y && \
yum install -y libcurl-devel zlib-devel clang python pkg-config git bzip2 unzip make wget sudo  && \
yum install -y zlib-static gmock gtest

CMD /bin/bash