#docker build -t avidemux/avidemux_mxe -f .devcontainer_mxe64/Dockerfile .
FROM debian:trixie

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_UID=1000
ARG USER_GID=1000
ARG USERNAME=mxe

#RUN  sed -i 's/deb.debian.org/192.168.0.9:3142\/deb.debian.org/g' /etc/apt/sources.list.d/debian.sources 
#
RUN sed -i '/^Components:/ s/$/ contrib non-free non-free-firmware/' /etc/apt/sources.list.d/debian.sources
#RUN sed -i '/^Components:/ s/$/ contrib non-free non-free-firmware/' /etc/apt/sources.list.d/debian-backports.sources
  #&& sed -i 's/deb.debian.org/192.168.0.9:3142\/deb.debian.org/g' /etc/apt/sources.list.d/debian-backports.sources \
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
    zip \
    build-essential pkg-config yasm \
    file  \
    desktop-file-utils fakeroot \
    fuse  patchelf python3-pip python3-setuptools squashfs-tools strace \
    util-linux zsync libfuse2t64  \
    zsh neovim \
      autoconf \
    automake \
    autopoint \
    bash \
    g++ \
    g++-multilib \
    gettext \
    intltool \
    libc6-dev-i386 \
    libclang-dev \
    libltdl-dev \
    libgl-dev \
    libpcre2-dev \
    libssl-dev \
    libtool-bin \
    libxml-parser-perl \
    lzip \
    openssl \
    p7zip-full \
    patch \
    perl \
    python3-mako \
    python3-packaging \
    python3-pkg-resources \
    python3-setuptools \
    python-is-python3 \
    sed \
    sqlite3 \
    apt-file \
    xz-utils \
    libgl-dev libgl1-mesa-dev \
    nsis \
    nsis-common \
    nsis-doc \
    nsis-pluginapi 

  RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
  && :
    #libaften-dev \

    #libgdk-pixbuf2.0-dev \
RUN useradd -m mxe && usermod --shell /usr/bin/zsh mxe
RUN chmod a+rwx /opt/
RUN echo 'root:password' | chpasswd 
#
RUN apt-get install -y python3-pefile

#
#

CMD [ "/bin/bash" ]

###################################
# Setup user env
###################################
USER ${USERNAME}
RUN mkdir ~/.local &&  mkdir ~/.local/bin &&  mkdir ~/.cargo && mkdir ~/.cargo/bin
RUN echo "export PATH=\$PATH:\$HOME/.cargo/bin:\$HOME/.local/bin"  >> ~/.bashrc
#
RUN cat <<EOF > /home/mxe/.zshrc
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
# Split the build in two to avoid timeouts, not sure that helps
RUN cd /opt &&  git clone https://github.com/mxe/mxe.git && cd mxe && \
     echo "MXE_TARGETS := x86_64-w64-mingw32.shared" > settings.mk \
    && echo "JOBS := 4" >> settings.mk && make  qt6-qtbase
RUN cd /opt/mxe && make qt6
RUN echo Done
ENV PATH="/opt/mxe/usr/bin:/usr/local/bin:/opt/mxe/bin:/opt/mxe/usr/x86_64-pc-linux-gnu/qt6/bin/:${PATH}"
# --- install libaom
#COPY ./.devcontainer_mxe64/install-libaom.bash /tmp/
COPY install-libaom.bash /tmp/
RUN cd /tmp && bash install-libaom.bash /opt/mxe



