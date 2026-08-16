FROM fedora:43
#RUN sed -i 's|^metalink=|#metalink=|g' /etc/yum.repos.d/*.repo
#RUN sed -i 's|^#baseurl=http://download.example/pub/fedora/linux|baseurl=http://mirror.in2p3.fr/pub/fedora/linux/|g' /etc/yum.repos.d/fedora.repo /etc/yum.repos.d/fedora-updates.repo  && \
#sed -i 's|^metalink=|#metalink=|g' /etc/yum.repos.d/fedora.repo
RUN rm -Rf  /etc/yum.repos.d/fedora-cisco-openh264.repo
RUN sudo dnf -y install \
  https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm \
  https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
RUN dnf -y install git shadow-utils findutils util-linux tar sshd wget desktop-file-utils \
  && useradd -m fedora
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

RUN dnf -y install \
  qt6-qtbase-devel \
  qt6-qtbase-private-devel \
  qt6-linguist
RUN dnf -y install SDL3-devel 
RUN dnf -y clean all

# We've already got ffnvcodec headers from Fedora repositories.
#RUN cd /tmp && git clone https://github.com/FFmpeg/nv-codec-headers.git && cd nv-codec-headers && make install

# App image
RUN wget --no-proxy https://github.com/AppImageCrafters/appimage-builder/releases/download/v1.1.0/appimage-builder-1.1.0-x86_64.AppImage \
  && chmod +x appimage-builder-1.1.0-x86_64.AppImage \
  && mv appimage-builder-1.1.0-x86_64.AppImage /usr/local/bin/appimage-builder

RUN wget --no-proxy https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O /usr/local/bin/linuxdeploy \
  && chmod +x /usr/local/bin/linuxdeploy

RUN wget --no-proxy https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage \
  -O /usr/local/bin/linuxdeploy-plugin-appimage \
  && wget --no-proxy https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage \
    -O /usr/local/bin/linuxdeploy-plugin-qt \
  && wget --no-proxy https://github.com/AppImage/type2-runtime/releases/download/continuous/runtime-x86_64 \
    -O /usr/local/bin/runtime-x86_64 \
  && wget --no-proxy https://github.com/AppImage/appimagetool/releases/download/1.9.0/appimagetool-x86_64.AppImage \
    -O /usr/local/bin/appimagetool \
  && chmod +x /usr/local/bin/*

WORKDIR /home/fedora/workspace
RUN chown fedora:fedora /home/fedora/workspace

USER fedora
CMD ["/bin/bash"]
