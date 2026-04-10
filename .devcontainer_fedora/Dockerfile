FROM fedora:43
RUN sed -i 's|^metalink=|#metalink=|g' /etc/yum.repos.d/*.repo
RUN sed -i 's|^#baseurl=http://download.example/pub/fedora/linux|baseurl=http://mirror.in2p3.fr/pub/fedora/linux/|g' /etc/yum.repos.d/fedora.repo /etc/yum.repos.d/fedora-updates.repo  && \
sed -i 's|^metalink=|#metalink=|g' /etc/yum.repos.d/fedora.repo 
RUN rm -Rf  /etc/yum.repos.d/fedora-cisco-openh264.repo
RUN sudo dnf -y install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
RUN dnf -y  install git shadow-utils findutils util-linux tar sshd && useradd -m fedora 
RUN dnf -y  install   --skip-unavailable gcc \
  gcc-c++ \
  make \
  cmake \
  yasm \
  pkgconf-pkg-config \
  fakeroot \
  bzip2 \
  zlib-ng-compat-devel \
  patch \
  rpm-build \
  sqlite-devel \
  libxslt \
  mesa-libGL-devel \
  mesa-libGLU-devel \
  libvdpau-devel \
  libva-devel \
  libXv-devel \
  libvorbis-devel \
  libogg-devel \
  alsa-lib-devel \
  pulseaudio-libs-devel \
  libass-devel \
  opus-devel \
  lame-devel \
  libvpx-devel \
  libaom-devel \
  nv-codec-headers \
  fdk-aac-devel \
  twolame-devel \
  xvidcore-devel \
  x264-devel \
  x265-devel
RUN dnf -y install qt6-*{devel,doc}*
RUN dnf -y clean all

RUN cd /tmp && git clone https://github.com/FFmpeg/nv-codec-headers.git && cd nv-codec-headers && make install


WORKDIR /home/fedora/workspace
RUN chown fedora:fedora /home/fedora/workspace

USER fedora
CMD ["/bin/bash"]
