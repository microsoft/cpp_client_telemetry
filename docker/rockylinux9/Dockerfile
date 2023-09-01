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

RUN \
  dnf -y install python3.9 python3-pip && \
  echo 'alias python="/bin/python3"' >> ~/.bashrc && \
  echo 'alias python="/bin/python3"' >> ~/.bash_profile && \
  pip3 install scons && \
  pip3 install distro && \
  ln -fs /bin/python3.9 /bin/python

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

# Boost
RUN \
  wget -nv -q https://boostorg.jfrog.io/artifactory/main/release/1.67.0/source/boost_1_67_0.tar.gz && \
  echo '8aa4e330c870ef50a896634c931adf468b21f8a69b77007e45c444151229f665 boost_1_67_0.tar.gz' | sha256sum -c && \
  tar -xzf boost_1_67_0.tar.gz && \
  cd boost_1_67_0 && \
  ./bootstrap.sh --prefix=/usr --libdir=/usr/lib64 cxxflags=-fPIC cflags=-fPIC link=static --with-libraries=chrono,random,system,thread,filesystem,atomic,date_time,regex && \
  ./b2 install --prefix=/usr --libdir=/usr/lib64 cxxflags=-fPIC cflags=-fPIC link=static -j$(nproc) && \
  cd .. && rm -f boost_1_67_0.tar.gz && rm -rf boost_1_67_0


# Swig
RUN \
  wget https://sourceforge.net/projects/swig/files/swig/swig-4.0.2/swig-4.0.2.tar.gz/download && \
  tar xzvf download && \
  cd swig-4.0.2 && \
  ./configure && \
  make && \
  make install && \
  cd .. && rm -f download && rm -rf swig-4.0.2

# Nuget
RUN \
  dnf install -y mono-devel mono-complete && \
  curl -o /usr/local/bin/nuget.exe https://dist.nuget.org/win-x86-commandline/latest/nuget.exe && \
  echo 'alias nuget="mono /usr/local/bin/nuget.exe"' >> ~/.bashrc && \
  echo 'alias nuget="mono /usr/local/bin/nuget.exe"' >> ~/.bash_profile

# Cmake
RUN \
  wget https://cmake.org/files/v3.24/cmake-3.24.1.tar.gz && \
  tar -xzvf cmake-3.24.1.tar.gz && \
  cd cmake-3.24.1 && \
  export OPENSSL_ROOT_DIR=/usr/mip && \
  ./bootstrap && \
  make -j$(nproc) && \
  make install && \
  cd .. && rm -f cmake-3.24.1.tar.gz && rm -rf cmake-3.24.1

# dotnet
RUN \
  rpm --import https://packages.microsoft.com/keys/microsoft.asc && \
  wget https://packages.microsoft.com/config/rhel/9.0/prod.repo -O /etc/yum.repos.d/microsoft-prod.repo && \
  dnf install -y openssl-devel && \
  dnf install -y dotnet-sdk-6.0 aspnetcore-runtime-6.0 dotnet-runtime-6.0

# For Agnostic testing
RUN \
  rm -rf /etc/ssl && \
  ln -s /etc/pki/tls /etc/ssl

EXPOSE 80
EXPOSE 443