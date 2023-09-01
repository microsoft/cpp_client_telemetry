FROM mipsdkcontainer.azurecr.io/library/rockylinux:9

# Install Dependencies
RUN \
  yum install -y yum-utils && \
  yum-config-manager --enable crb && \
  yum -y update && \
  yum -y install epel-release && \
  yum -y group install "Development Tools" && \
  yum -y install git libgsf libgsf-devel libxml2-devel libcurl-devel wget gdk-pixbuf2 glib2-devel libuuid-devel && \
  yum -y install libsecret libsecret-devel gnome-keyring

# openssl
ENV OPENSSL_VERSION 1_1_1r
RUN \
  dnf install -y perl-FindBin && \
  dnf install -y perl-File-Compare && \
  curl -O -L https://github.com/openssl/openssl/archive/refs/tags/OpenSSL_$OPENSSL_VERSION.tar.gz && \
  echo "fc513913724790510f53af07caa24eaf0eae3fc8cf476c17c113221b5868edac OpenSSL_$OPENSSL_VERSION.tar.gz" | sha256sum -c && \
  tar -xzvf OpenSSL_$OPENSSL_VERSION.tar.gz && \
  cd openssl-OpenSSL_$OPENSSL_VERSION && \
  ./config --prefix=/usr/mip --libdir=/usr/mip/lib --openssldir=/etc/ssl no-shared no-ssl2 no-ssl3 no-comp zlib-dynamic enable-ec_nistp_64_gcc_128 && \
  make -j$(nproc) && \
  make install_sw && \
  cd .. && rm -f OpenSSL_$OPENSSL_VERSION.tar.gz && rm -rf openssl-OpenSSL_$OPENSSL_VERSION

EXPOSE 80
EXPOSE 443