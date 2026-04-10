FROM debian:trixie

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_UID=1000
ARG USER_GID=1000
ARG USERNAME=trixie

#RUN echo 'Acquire::http::Proxy "http://192.168.0.9:3142";' > /etc/apt/apt.conf.d/01proxy
RUN sed -i '/^Components:/ s/$/ contrib non-free non-free-firmware/' /etc/apt/sources.list.d/debian.sources
RUN cat <<EOF > /etc/apt/sources.list.d/debian-backports.sources
Types: deb
URIs: https://deb.debian.org/debian
Suites: trixie-backports
Components: main contrib non-free non-free-firmware 
Signed-By: /usr/share/keyrings/debian-archive-keyring.gpg
EOF

RUN cat <<EOF > /etc/apt/preferences.d/99-backports.pref
Package: *
Pin: release a=trixie-backports
Pin-Priority: 500
EOF

RUN apt-get update 
RUN apt-get install -y \
    apt-utils \
    bison \
    build-essential \
    bzip2 \
    ca-certificates \
    ccache \
    check \
    curl \
    cmake \
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
    build-essential pkg-config yasm \
    libsqlite3-dev \
    libxv-dev libvdpau-dev libva-dev libglu1-mesa-dev \
    libasound2-dev libpulse-dev \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-base-dev-tools \
    qt6-l10n-tools \
    qt6-wayland \
    libx264-dev libxvidcore-dev \
    libmp3lame-dev libtwolame-dev libopus-dev libvorbis-dev libogg-dev \
    libpng-dev libass-dev 

  RUN apt-get install  -y \
    libfaac-dev \
    libfdk-aac-dev \
    libx265-dev \
    libvpx-dev \
    libaom-dev \
    file \
    squashfs-tools 
  # app image
  RUN apt-get install -y  desktop-file-utils fakeroot fuse  patchelf python3-pip python3-setuptools squashfs-tools strace util-linux zsync libfuse2t64
  RUN apt-get install -y  zsh neovim

  RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
  && :
    #libaften-dev \
# Missing packages
  RUN apt-get install  -y \
    libopencore-amrnb-dev \
    libopencore-amrwb-dev \
    libdca-dev \
    libfaad-dev  

    #libaften-dev

# App image
RUN wget --no-proxy https://github.com/AppImageCrafters/appimage-builder/releases/download/v1.1.0/appimage-builder-1.1.0-x86_64.AppImage && \
chmod +x appimage-builder-1.1.0-x86_64.AppImage && \
  mv appimage-builder-1.1.0-x86_64.AppImage /usr/local/bin/appimage-builder

RUN wget --no-proxy https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O /usr/local/bin/linuxdeploy && \
      chmod +x /usr/local/bin/linuxdeploy

RUN  wget --no-proxy https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage \
    -O /usr/local/bin/linuxdeploy-plugin-appimage  && \
    wget --no-proxy https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage   \
    -O /usr/local/bin/linuxdeploy-plugin-qt  && \
    wget --no-proxy https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64   \
    -O /usr/local/bin/runtime-x86_64   && \
    wget --no-proxy https://github.com/AppImage/appimagetool/releases/download/1.9.0/appimagetool-x86_64.AppImage \
    -O /usr/local/bin/appimagetool && \
    chmod +x /usr/local/bin/*
RUN useradd -m trixie && usermod --shell /usr/bin/zsh trixie
#
RUN cd /tmp && git clone https://github.com/FFmpeg/nv-codec-headers.git && cd nv-codec-headers && make install
# Cleanup
#RUN apt autoclean && apt autoremove
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
RUN cat <<EOF > /home/trixie/.zshrc
# History Settings
HISTFILE=~/.zsh_history
HISTSIZE=10000
SAVEHIST=10000
setopt appendhistory
setopt sharehistory

# Completion System
autoload -Uz compinit && compinit

# Prompt
PROMPT='%F{cyan}[%n@docker]%f %F{blue}%~%f %# '

# Aliases
alias ls='ls --color=auto'
alias ll='ls -lah'
EOF



