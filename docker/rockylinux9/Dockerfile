FROM mipsdkcontainer.azurecr.io/rockylinux9

## Common packages for linux build environment
RUN yum update -y && \
yum install -y libcurl-devel zlib-devel clang python pkg-config git bzip2 unzip make wget sudo cmake && \
yum install -y zlib-static gmock gtest

CMD /bin/bash