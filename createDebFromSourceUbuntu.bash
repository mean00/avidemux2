#!/bin/bash
echo "This will install all the necessary packages and build avidemux, it will take 20 mn or so"
echo "You will be asked to enter your password because installing build dependencies requires root permissions"
# gcc, g++ and make get installed as dependencies of build-essential
sudo apt-get update && sudo apt-get install build-essential cmake pkg-config yasm \
libsqlite3-dev libfontconfig1-dev libfribidi-dev libxv-dev libvdpau-dev libpulse-dev \
qttools5-dev-tools qtbase5-dev libqt5opengl5-dev \
libpng12-dev libaften-dev libmp3lame-dev libx264-dev libfaad-dev libfaac-dev libopus-dev libvorbis-dev libogg-dev libdca-dev \
 || { echo "The installation at least of some of the build dependencies failed. Aborting." && exit 2; }
sudo apt-get install libx265-dev \
 || echo "Warning: libx265-dev cannot be installed using package management. Avidemux won't be able to encode in h265 unless the library and the headers have been installed manually. Continuing anyway." # there are no official libx265 packages for Ubuntu Trusty
# 
echo "Compiling avidemux"
bash bootStrap.bash --deb || { echo "Build failed. Cancelling installation." && exit 3; }
echo "Installing avidemux..."
cd debs && sudo dpkg -i *
exit 0
