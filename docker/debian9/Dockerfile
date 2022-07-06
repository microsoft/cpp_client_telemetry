FROM mipsdkcontainer.azurecr.io/debian9

ENV DEBIAN_FRONTEND noninteractive


## Common packages for linux build environment
RUN apt install -y clang python pkg-config git curl nghttp2 bzip2 unzip make wget cmake sudo

RUN adduser --disabled-password --gecos '' docker
RUN adduser docker sudo
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
## RUN useradd -m docker && echo "docker:docker" | chpasswd && adduser docker sudo
## USER docker

CMD /bin/bash

# ENTRYPOINT bash
