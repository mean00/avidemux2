#!/bin/bash
echo "Automatic RPM generator for Avidemux, Fedora 29 version"
#
ID=$(id -u)
if [ "x${ID}" = "x0" ]; then
    echo "Don't build Avidemux as root, aborting"
    exit 1
fi

# build dependencies
PKGLIST="gcc \
gcc-c++ \
make \
cmake \
yasm \
pkgconf-pkg-config \
fakeroot \
bzip2 \
libpng \
zlib-devel \
patch \
rpm-build \
sqlite-devel \
qt5-qtbase-devel \
qt5-linguist \
libxslt \
mesa-libGL-devel \
mesa-libGLU-devel \
libvdpau-devel \
libva-devel \
libXv-devel \
libvorbis-devel \
libogg-devel \
pulseaudio-libs-devel \
fribidi-devel \
fontconfig-devel \
freetype-devel \
opus-devel \
nv-codec-headers"

# optional features
OPTIONAL_PACKAGES="lame-devel \
fdk-aac-devel \
twolame-devel \
xvidcore-devel \
x264-devel \
x265-devel"
#
for i in ${PKGLIST}; do
    rpm -q $i > /dev/null
    PRESENT=$?;
    if [ "x${PRESENT}" = "x1" ]; then
        REQUIREMENTS+=($i)
    fi
done
#
for i in ${OPTIONAL_PACKAGES}; do
    rpm -q $i > /dev/null
    PRESENT=$?;
    if [ "x${PRESENT}" = "x1" ]; then
        EXTRAS+=($i)
    fi
done
#
NB=${#REQUIREMENTS[@]}
RESULT="0"
SUCOMMAND=""
if [ "x${NB}" != "x0" ]; then
    echo "Missing required development packages:"
    echo ${REQUIREMENTS[*]}
    read -p "Shall su or sudo be used to run commands with root privileges? " input
    if [ "x${input}" = "xsu" ]; then
        SUCOMMAND="su"
        su -c "/usr/bin/dnf install ${REQUIREMENTS[*]}"
        RESULT=$?
    elif [ "x${input}" = "xsudo" ]; then
        SUCOMMAND="sudo"
        sudo /usr/bin/dnf install ${REQUIREMENTS[*]}
        RESULT=$?
    else
        echo "Can't parse the input, bye"
        exit 1
    fi
fi
#
if [ "x${RESULT}" != "x0" ]; then
    echo "Failed to install all required packages, aborting"
    exit 1
fi
#
NB=${#EXTRAS[@]}
#
if [ "x${NB}" != "x0" ]; then
    echo "Some useful optional development packages are missing:"
    echo ${EXTRAS}
    input="N"
    skip_extras="0"
    echo "RPM Fusion repositories must be available on this system to install these packages."
    if [ "x${SUCOMMAND}" = "x" ]; then
        read -p "Shall su or sudo be used to run commands with root privileges? " input
        if [ "x${input}" = "xsu" ]; then
            SUCOMMAND="su"
        elif [ "x${input}" = "xsudo" ]; then
            SUCOMMAND="sudo"
        else
            skip_extras="1"
            echo "Can't parse input, skipping."
        fi
    fi
    if [ "x${skip_extras}" = "x0" ]; then
        read -p "Shall these packages be installed too? Type Y to install. " input
        if [ "x${input}" = "xY" ]; then
            if [ "x${SUCOMMAND}" = "xsu" ]; then
                su -c "/usr/bin/dnf install ${EXTRAS[*]}"
            elif [ "x${SUCOMMAND}" = "xsudo" ]; then
                sudo /usr/bin/dnf install ${EXTRAS[*]}
            fi
        else
            echo "Skipped."
        fi
    fi
fi
#
echo "Building..."
umask 0022
bash bootStrap.bash --rpm
RESULT=$?
if [ "x${RESULT}" = "x0" ]; then
    echo "All done, the RPMS are in the debs folder"
else
    echo "Something went wrong, please inspect /tmp/logbuil* files"
fi
exit ${RESULT}
