FROM ubuntu:26.04

ARG DEBIAN_FRONTEND=noninteractive
ARG USER_UID=1000
ARG USER_GID=1000
ARG USERNAME=ubuntu

#RUN sed -i 's|http://archive.ubuntu.com|http://ports.ubuntu.com|g' /etc/apt/sources.list && \
    #apt-get update 

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
  && apt-get install -y libsdl3-dev \
  && update-alternatives --install /usr/bin/python python /usr/bin/python3 10 \
  && :

RUN cd /tmp && git clone https://github.com/FFmpeg/nv-codec-headers.git && cd nv-codec-headers && make install


#
#  Reuse Arm & Riscv toolchains
#
#
RUN apt-get install -y zsh neovim apt-file 
#
#
RUN chsh  -s /usr/bin/zsh ubuntu
RUN usermod -aG sudo ubuntu
#
RUN rm -f  /etc/apt/apt.conf.d/01proxy
RUN apt-file update
#RUN rm -rf /var/lib/apt/lists/* 
#  && apt-get autoremove -y \
#  && rm -rf /var/lib/apt/lists/* \

CMD [ "/bin/bash" ]

###################################
# Setup user env
###################################
#
USER ${USERNAME}
RUN mkdir ~/.local &&  mkdir ~/.local/bin &&  mkdir ~/.cargo && mkdir ~/.cargo/bin
#
RUN cat <<EOF > /home/ubuntu/.zshrc
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
export TERM='linux'
EOF
#

