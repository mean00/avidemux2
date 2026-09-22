FROM debian:bullseye

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_UID=1000
ARG USER_GID=1000
ARG USERNAME=bullseye

#RUN echo 'Acquire::http::Proxy "http://192.168.0.9:3142";' > /etc/apt/apt.conf.d/01proxy
# bullseye reached end-of-LTS on 2026-08-31 and Debian deleted the whole
# bullseye-security *pool*: the dists/ index is still served, but every .deb it
# points at now returns 404 (libc6-dev, libcurl4, openssl, libexpat1, cmake-latest,
# ...), so plain "apt-get install" fails in this image.  Note this is not only about
# cmake-latest: the image ships the LTS-updated runtime libraries (glibc
# 2.31-13+deb11u14, ...) and their matching -dev packages only ever existed in
# bullseye-security, so the suite cannot just be dropped - apt then reports
# "libc6-dev : Depends: libc6 (= 2.31-13+deb11u11) but 2.31-13+deb11u14 is to be
# installed" and refuses to downgrade.
# The suite is therefore served from snapshot.debian.org at the timestamp this base
# image was built from (debuerreotype records it in the image's own sources.list),
# which gives apt exactly the versions that are already installed.  The archived
# Release file is expired forever, hence check-valid-until=no.
# Everything else comes from the live archive; when deb.debian.org retires bullseye,
# switch those two lines to http://archive.debian.org/debian (no Valid-Until there).
RUN printf '%s\n' \
      'deb http://deb.debian.org/debian bullseye main contrib non-free' \
      'deb http://deb.debian.org/debian bullseye-updates main contrib non-free' \
      'deb [check-valid-until=no] http://snapshot.debian.org/archive/debian-security/20260824T000000Z bullseye-security main' \
      > /etc/apt/sources.list
RUN echo "deb http://archive.debian.org/debian bullseye-backports main contrib non-free" > /etc/apt/sources.list.d/bullseye-backports.list
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
    cmake-latest \
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
    libx264-dev libxvidcore-dev \
    libmp3lame-dev libtwolame-dev libopus-dev libvorbis-dev libogg-dev \
    libpng-dev libass-dev \
    file \
    squashfs-tools 
    # app image
  RUN apt-get install -y  desktop-file-utils fakeroot fuse  patchelf python3-pip python3-setuptools squashfs-tools strace util-linux zsync 
  RUN apt-get install -y  zsh neovim nasm

  RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
  && :
    #libaften-dev \

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
    -O /usr/local/bin/appimagetool1 && \
    chmod +x /usr/local/bin/*
RUN useradd -m bullseye && usermod --shell /usr/bin/zsh bullseye
RUN chmod a+rwx /opt /usr/local/bin
#
#

# 
# 
# 
# Extra packages
# 
# 
# 
RUN  apt-get install  -y \
    libx264-dev \
    libx265-dev \
    libvpx-dev \
    libaom-dev \
    libfuse-dev \
    libfaad-dev  \
    libopencore-amrnb-dev \
    libopencore-amrwb-dev

RUN  apt-get install  -y \
    qt6-base-dev qt6-l10n-tools qt6-wayland

    #libfaac-dev \
    #libfdk-aac-dev \
#libfuse2t64

# 
CMD [ "/bin/bash" ]


RUN cd /tmp && git clone https://github.com/FFmpeg/nv-codec-headers.git && cd nv-codec-headers && make install

###################################
# Setup user env
###################################
USER ${USERNAME}
#
RUN cd /tmp  && /usr/local/bin/appimagetool1 --appimage-extract && mv squashfs-root /opt/appimagetool-extracted 
#
RUN cat <<EOF > /home/bullseye/.zshrc
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
# give access to image tool
ENV PATH="/opt/appimagetool-extracted/usr/bin/:$PATH"



