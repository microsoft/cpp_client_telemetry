FROM mipsdkcontainer.azurecr.io/library/rockylinux:9

# Install Dependencies
RUN \
  yum install -y yum-utils && \
  yum-config-manager --enable crb && \
  yum -y update && \
  yum -y install epel-release && \
  yum -y group install "Development Tools" && \
  yum -y install git libgsf libgsf-devel libxml2-devel libcurl-devel wget gdk-pixbuf2 glib2-devel libuuid-devel perl-IPC-Cmd && \
  yum -y install libsecret libsecret-devel gnome-keyring

# openssl
ENV OPENSSL_VERSION 3.0.9
RUN \
  dnf install -y perl-FindBin && \
  dnf install -y perl-File-Compare && \
  curl -O -L https://github.com/openssl/openssl/archive/refs/tags/openssl-$OPENSSL_VERSION.tar.gz && \
  echo "2eec31f2ac0e126ff68d8107891ef534159c4fcfb095365d4cd4dc57d82616ee openssl-$OPENSSL_VERSION.tar.gz" | sha256sum -c && \
  tar -xzvf openssl-$OPENSSL_VERSION.tar.gz && \
  cd openssl-openssl-$OPENSSL_VERSION && \
  ./config --prefix=/usr/mip --libdir=/usr/mip/lib --openssldir=/etc/ssl no-shared no-ssl2 no-ssl3 no-comp zlib-dynamic enable-ec_nistp_64_gcc_128 && \
  make -j$(nproc) && \
  make install_sw && \
  cd .. && rm -f openssl-$OPENSSL_VERSION.tar.gz && rm -rf openssl-openssl-$OPENSSL_VERSION

CMD /bin/bash