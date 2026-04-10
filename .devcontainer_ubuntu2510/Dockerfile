FROM ubuntu:25.10

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_UID=1000
ARG USER_GID=1000
ARG USERNAME=ubuntu

#RUN echo 'Acquire::http::Proxy "http://192.168.0.9:3142";' > /etc/apt/apt.conf.d/01proxy

RUN apt-get update 
RUN apt-get install -y \
    apt-utils \
    bison \
    build-essential \
    bzip2 \
    ca-certificates \
    ccache \
    check \
    cmake \
    curl \
    flex \
    git \
    git-lfs \
    gperf \
    lcov \
    libbsd-dev \
    libffi-dev \
    libglib2.0-0 \
    libncurses-dev \
    libpixman-1-0 \
    libsdl2-2.0-0 \
    libslirp0 \
    libusb-1.0-0-dev \
    make \
    ninja-build \
    python3 \
    python3-venv \
    python3-crcmod \
    ruby \
    sudo \
    unzip \
    usbutils \
    wget \
    xz-utils \
    zip \
    build-essential cmake pkg-config yasm \
    libsqlite3-dev \
    libxv-dev libvdpau-dev libva-dev libglu1-mesa-dev \
    libasound2-dev libpulse-dev \
    qt6-base-dev qt6-l10n-tools \
    libx264-dev libxvidcore-dev \
    libmp3lame-dev libtwolame-dev libopus-dev libvorbis-dev libogg-dev \
    libpng-dev libass-dev \
    libaften-dev \
    libfaac-dev \
    libfdk-aac-dev \
    libx265-dev \
    libvpx-dev \
    libaom-dev \
    file \
  && apt-get autoremove -y \
  && rm -rf /var/lib/apt/lists/* \
  && update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
  && :

RUN cd /tmp && git clone https://github.com/FFmpeg/nv-codec-headers.git && cd nv-codec-headers && make install


RUN rm -f  /etc/apt/apt.conf.d/01proxy
#
#  Reuse Arm & Riscv toolchains
#

CMD [ "/bin/bash" ]

###################################
# Setup user env
###################################
USER ${USERNAME}
RUN mkdir ~/.local &&  mkdir ~/.local/bin &&  mkdir ~/.cargo && mkdir ~/.cargo/bin
RUN echo "export http_proxy=\"\"">> ~/.bashrc
RUN echo "export https_proxy=\"\"">> ~/.bashrc
RUN echo "export HTTP_PROXY=\"\"">> ~/.bashrc
RUN echo "export HTTPS_PROXY=\"\"">> ~/.bashrc
RUN echo "export PATH=\$PATH:\$HOME/.cargo/bin:\$HOME/.local/bin"  >> ~/.bashrc
RUN echo ".  \$HOME/.cargo/env" >> /home/$USERNAME/.bashrc
ENV http_proxy=""
ENV https_proxy=""
ENV HTTP_PROXY=""
ENV HTTPS_PROXY=""
#


