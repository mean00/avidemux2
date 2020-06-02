#!/bin/bash
install_packages=1
rebuild=""
sucommand=""
missing_pkgs=()
# build dependencies
PKGLIST="gcc \
gcc-c++ \
make \
cmake \
yasm \
pkgconf-pkg-config \
fakeroot \
bzip2 \
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
alsa-lib-devel \
pulseaudio-libs-devel \
fribidi-devel \
fontconfig-devel \
freetype-devel \
opus-devel \
lame-devel \
libvpx-devel \
libaom-devel \
nv-codec-headers"
# optional features
OPTIONAL_PACKAGES="fdk-aac-devel \
twolame-devel \
xvidcore-devel \
x264-devel \
x265-devel"
#
usage()
{
    echo "Usage: $0 [Options]"
    echo "***********************"
    echo "  --help or -h      : Print usage"
    echo "  --su              : Don't prompt, use su to run commands as root"
    echo "  --sudo            : Don't prompt, use sudo to run commands as root"
    echo "  --deps-only       : Just install build dependencies and exit"
    echo "  --no-install      : Don't install generated RPM packages"
    echo "  --rebuild         : Preserve existing build directories"
}
#
fail()
{
   echo "$1" && exit 1
}
#
check_deps()
{
    for i in $@; do
        rpm -q $i > /dev/null
        notfound=$?;
        if [ ${notfound} -eq 1 ]; then
            missing_pkgs+=($i)
        fi
    done
}
#
get_su_command()
{
    if [ "x${sucommand}" != "x" ]; then
        return 0
    fi
    read -p "Shall su or sudo be used to run commands with root privileges? " input
    if [ "x${input}" = "xsu" ]; then
        sucommand="su"
    elif [ "x${input}" = "xsudo" ]; then
        sucommand="sudo"
    else
        echo "Can't parse the input."
        return 1
    fi
}
#
install_deps()
{
    check_deps ${PKGLIST}
    missing_required=(${missing_pkgs[*]})
    missing_pkgs=()
    check_deps ${OPTIONAL_PACKAGES}
    missing_extras=(${missing_pkgs[*]})
    missing_pkgs=()
    if [ ${#missing_required[@]} -gt 0 ]; then
        echo "Missing required development packages:"
        echo ${missing_required[*]}
        get_su_command || exit 1
        message="Failed to install all required packages, aborting."
        if [ "x${sucommand}" = "xsu" ]; then
            su -c "/usr/bin/dnf install ${missing_required[*]}" || fail ${message}
        elif [ "x${sucommand}" = "xsudo" ]; then
            sudo /usr/bin/dnf install ${missing_required[*]} || fail ${message}
        fi
    fi
    if [ ${#missing_extras[@]} -gt 0 ]; then
        echo "Some useful optional development packages are missing."
        echo "RPM Fusion repositories must be already configured and enabled on this system to install them."
        echo ${missing_extras[*]}
        read -p "Shall these packages be installed too? Type uppercase Y to install. " input
        if [ "x${input}" = "xY" ]; then
            get_su_command || { echo "Skipping optional packages." && return 0; }
            if [ "x${sucommand}" = "xsu" ]; then
                su -c "/usr/bin/dnf install ${missing_extras[*]}" || return 0
            elif [ "x${sucommand}" = "xsudo" ]; then
                sudo /usr/bin/dnf install ${missing_extras[*]} || return 0
            fi
        else
            echo "Skipped."
        fi
    fi
}
#
install_avidemux()
{
    echo "Installing Avidemux..."
    get_su_command || fail "Can't install Avidemux."
    if $(rpm -qa | grep -q avidemux); then
        echo "Uninstalling previously installed Avidemux packages."
        message="Can't remove installed Avidemux packages, aborting."
        if [ "x${sucommand}" = "xsu" ]; then
            su -c '/usr/bin/dnf remove "avidemux*"' || fail ${message}
        elif [ "x${sucommand}" = "xsudo" ]; then
            sudo /usr/bin/dnf remove "avidemux*" || fail ${message}
        fi
    fi
    cd debs || fail "debs folder not present in the current directory, aborting."
    message="Installation failed."
    if [ "x${sucommand}" = "xsu" ]; then
        su -c "/usr/bin/dnf install avidemux*.rpm" || fail ${message}
    elif [ "x${sucommand}" = "xsudo" ]; then
        sudo /usr/bin/dnf install avidemux*.rpm || fail ${message}
    fi
}
#
echo "Automatic RPM generator for Avidemux, Fedora 29 version"
ID=$(id -u)
if [ "x${ID}" = "x0" ]; then
    fail "Don't build Avidemux as root, aborting"
fi
#
while [ $# != 0 ]; do
    config_option="$1"
    case "${config_option}" in
        -h|--help)
            usage
            exit 0
            ;;
        --deps-only)
            install_deps
            exit 0
            ;;
        --su)
            sucommand="su"
            ;;
        --sudo)
            sucommand="sudo"
            ;;
        --no-install)
            install_packages=0
            ;;
        --rebuild)
            rebuild="${config_option}"
            ;;
        *)
            echo "unknown parameter ${config_option}"
            usage
            exit 1
            ;;
    esac
    shift
done
#
install_deps
#
echo "Building..."
umask 0022
logfile="/tmp/log-bootstrap-$(date +%F_%T).log"
bash bootStrap.bash ${rebuild} --rpm 2>&1 | tee ${logfile}
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    fail "Build failed, please inspect ${logfile} and /tmp/logbuild* files."
fi
#
echo "Build completed, the RPMS are in the debs folder"
#
if [ ${install_packages} -eq 1 ]; then
    install_avidemux
fi
#
exit 0
