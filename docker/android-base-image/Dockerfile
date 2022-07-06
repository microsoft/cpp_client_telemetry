FROM mipsdkcontainer.azurecr.io/ubuntu:bionic
RUN dpkg --add-architecture i386
RUN apt-get update -qq
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y openjdk-8-jdk libc6:i386 libstdc++6:i386 libgcc1:i386 libncurses5:i386 libz1:i386 net-tools
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y wget unzip
ENV ANDROID_HOME /opt/android-sdk-linux

# Download sdk tools, add to path
RUN cd /opt \
  && wget -q https://dl.google.com/android/repository/sdk-tools-linux-4333796.zip -O android-sdk-tools.zip \
  && unzip -q android-sdk-tools.zip -d ${ANDROID_HOME} \
  && rm android-sdk-tools.zip
ENV PATH ${PATH}:${ANDROID_HOME}/tools:${ANDROID_HOME}/tools/bin:${ANDROID_HOME}/platform-tools

RUN yes | sdkmanager  --licenses
RUN touch /root/.android/repositories.cfg
RUN yes | sdkmanager \
  "platforms;android-29" \
  "build-tools;29.0.2" \
  "system-images;android-29;google_apis;x86" \
  "system-images;android-28;google_apis;x86" \
  "system-images;android-26;google_apis;x86"
RUN apt-get update \
  && apt-get -y install gradle \
  && gradle -v

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y libqt5widgets5
ENV QT_QPA_PLATFORM offscreen
ENV LD_LIBRARY_PATH ${ANDROID_HOME}/tools/lib64:${ANDROID_HOME}/emulator/lib64:${ANDROID_HOME}/emulator/lib64/qt/lib

RUN apt-get clean

# NDK
ENV ANDROID_NDK_HOME /opt/android-ndk
ENV ANDROID_NDK_VERSION r20

# download
RUN mkdir /opt/android-ndk-tmp && \
  cd /opt/android-ndk-tmp && \
  wget -q https://dl.google.com/android/repository/android-ndk-${ANDROID_NDK_VERSION}-linux-x86_64.zip && \
# uncompress
  unzip -q android-ndk-${ANDROID_NDK_VERSION}-linux-x86_64.zip && \
# move to its final location
  mv ./android-ndk-${ANDROID_NDK_VERSION} ${ANDROID_NDK_HOME} && \
# remove temp dir
  cd ${ANDROID_NDK_HOME} && \
  rm -rf /opt/android-ndk-tmp

# add to PATH
ENV PATH ${PATH}:${ANDROID_NDK_HOME}

# CMake and Ninja
ENV CMAKE_HOME /opt/cmake
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.16.3/cmake-3.16.3-Linux-x86_64.tar.gz
RUN wget -q https://github.com/ninja-build/ninja/releases/download/v1.10.0/ninja-linux.zip
RUN tar xf cmake-3.16.3-Linux-x86_64.tar.gz
RUN unzip ninja-linux.zip
RUN mv cmake-3.16.3-Linux-x86_64 /opt/cmake
RUN mv ninja "${CMAKE_HOME}/bin"
ENV PATH="${PATH}:${CMAKE_HOME}/bin"
RUN rm cmake-3.16.3-Linux-x86_64.tar.gz ninja-linux.zip
COPY ./assemble.sh .
RUN pwd
RUN chmod +x assemble.sh
RUN apt-get clean
CMD ["/bin/sh", "-c", "/assemble.sh"]