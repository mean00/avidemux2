#
echo "Automatic RPM generator for avidemux --Fedora version--"
echo "Please enter root password so i can install the development packages "
su -c "/usr/bin/dnf install gcc gcc-c++ make cmake yasm pkgconfig fakeroot yasm libpng zlib zlib-devel patch rpm-build libsqlite3x-devel  qt5-qtbase-devel qt5-qttools-devel libxslt mesa-libGLU-devel libvdpau-devel libXv-devel libvorbis-devel libogg-devel pulseaudio-libs-devel fribidi-devel fontconfig-devel freetype-devel opus-devel  " 
echo "Building..."
bash bootStrap.bash  --rpm
echo "All done, the RPMS are in the debs folder"
