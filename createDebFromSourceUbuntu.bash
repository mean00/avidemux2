#!/bin/bash
echo "This will install all the necessary packages and build avidemux, it will take 20 mn or so"
echo "Please enter your password"
sudo apt-get install gcc g++ make cmake pkg-config libpng12-dev fakeroot yasm libsqlite3-dev  build-essential  qttools5-dev-tools qtbase5-dev libaften-dev libmp3lame-dev libx264-dev  libfaad-dev libfaac-dev libvdpau-dev libx265-dev libopus-dev libvorbis-dev libogg-dev libfribidi-dev  \
libfontconfig1-dev  \
libxv-dev \
libpulse-dev \
libdca-dev
# 
echo "Compiling avidemux"
bash bootStrap.bash --deb
echo "Installing avidemux..."
cd debs && sudo dpkg -i *

