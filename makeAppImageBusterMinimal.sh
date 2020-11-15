#!/bin/bash
rebuild=""
missing_pkgs=()
# build dependencies
BUILD_DEPS="build-essential \
cmake \
pkg-config \
yasm \
libsqlite3-dev \
libxv-dev \
libvdpau-dev \
libva-dev \
libasound2-dev \
libpulse-dev \
qtbase5-dev \
qttools5-dev-tools \
libaom-dev \
libx264-dev \
libx265-dev \
libxvidcore-dev \
libvpx-dev \
libmad0-dev \
libmp3lame-dev \
libtwolame-dev \
libopus-dev \
libvorbis-dev \
libogg-dev \
libass-dev \
squashfs-tools \
wget"

NONFREE_PACKAGES="libfaac-dev \
libfdk-aac-dev"
#
usage()
{
    echo "Usage: $0 [Options]"
    echo "***********************"
    echo "  --help or -h      : Print usage"
    echo "  --setup-only      : Install build dependencies and exit"
    echo "  --rebuild         : Preserve existing build directories"
}
#
fail()
{
    echo "FAIL $@"
    exit 1
}
#
check_nvenc()
{
    if (pkg-config --exists ffnvcodec); then
        return 0
    fi
    echo "nv-codec-headers are missinng, will try to install."
    rm -rf /tmp/nvenc > /dev/null 2>&1
    mkdir /tmp/nvenc || return 1
    cd /tmp/nvenc || return 1
    git clone https://github.com/FFmpeg/nv-codec-headers.git || return 1
    cd nv-codec-headers || return 1
    # Get the most recent version still compatible with NVIDIA drivers
    # from the non-free repo on Buster.
    git checkout sdk/9.0 || return 1
    make || return 1
    sudo make install || return 1
}
#
check_deps()
{
    for i in $@; do
        state=$(dpkg -l $i 2>/dev/null | tail -n 1 | cut -d ' ' -f 1)
        if [ "${state}" != "ii" ] && [ "${state}" != "hi" ]; then
            missing_pkgs+=($i)
        fi
    done
}
#
setup()
{
    check_deps ${BUILD_DEPS}
    missing_required=(${missing_pkgs[*]})
    missing_pkgs=()
    if [ ${#missing_required[@]} -gt 0 ]; then
        echo "Missing required development packages:"
        echo ${missing_required[*]}
    fi
    check_deps ${NONFREE_PACKAGES}
    missing_nonfree=(${missing_pkgs[*]})
    missing_pkgs=()
    if [ ${#missing_nonfree[@]} -gt 0 ]; then
        echo "Missing non-free development packages:"
        echo ${missing_nonfree[*]}
        echo "Warning, non-free repositories must be already enabled on this system to install them."
    fi
    nb_missing=$(( ${#missing_required[@]} + ${#missing_nonfree[@]} ))
    if [ ${nb_missing} -gt 0 ]; then
        sudo /usr/bin/apt-get update || fail "Cannot sync repo metadata"
        sudo /usr/bin/apt-get install ${missing_required[*]} ${missing_nonfree[*]} || fail "Failed to install all build dependencies, aborting."
    fi
    if (check_nvenc); then
        echo "NVENC headers found."
    else
        echo "Cannot install NVENC headers."
    fi
}
#
echo "Automatic AppImage generator for Avidemux, Debian 10.6 version"
ID=$(id -u)
if [ "x${ID}" = "x0" ]; then
    fail "Won't run as root, aborting."
fi

while [ $# != 0 ]; do
    config_option="$1"
    case "${config_option}" in
        -h|--help)
            usage
            exit 0
            ;;
        --setup-only)
            setup
            exit 0
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

setup

ORG=$PWD
RUNTIME="runtime-x86_64"
RT_DIR="externalBinaries/AppImageKit"
SHA256SUM="24da8e0e149b7211cbfb00a545189a1101cb18d1f27d4cfc1895837d2c30bc30" # size: 188392 bytes
TO_CHECK=""
DO_DOWNLOAD=0
echo "Current directory: ${ORG}"
if [ ! -e "${ORG}/${RT_DIR}" ]; then
    mkdir -p "${RT_DIR}" || exit 1
fi
cd "${RT_DIR}" || exit 1
if [ -f "${ORG}/${RT_DIR}/${RUNTIME}" ]; then
    TO_CHECK=$(sha256sum "${RUNTIME}" | cut -d ' ' -f 1)
    if [ "${SHA256SUM}" != "${TO_CHECK}" ]; then
        echo "Checksum doesn't match, will try to re-download."
        rm -f "${RUNTIME}" > /dev/null 2>&1
        DO_DOWNLOAD=1
    else
        echo "${RUNTIME} has passed the check."
    fi
else
    echo "AppImageKit runtime is missing, will try to download."
    DO_DOWNLOAD=1
fi
if [ "x${DO_DOWNLOAD}" = "x1" ]; then
    URL="https://github.com/AppImage/AppImageKit/releases/download/12/runtime-x86_64"
    wget -O "${RUNTIME}" "${URL}" || exit 1
    TO_CHECK=$(sha256sum "${RUNTIME}" | cut -d ' ' -f 1)
fi
cd "${ORG}"
if [ "${SHA256SUM}" != "${TO_CHECK}" ]; then
    echo "Checksum doesn't match, aborting."
    exit 1
fi

rm -rf install > /dev/null 2>&1

export CXXFLAGS="$CXXFLAGS -std=c++11"
logfile="/tmp/log-bootstrap-$(date +%F_%T).log"
bash bootStrap.bash --with-system-libass --with-system-libmad ${rebuild} 2>&1 | tee ${logfile}
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    fail "Build failed, please inspect ${logfile} and /tmp/logbuild* files."
fi
bash appImage/deployBusterMinimal.sh "${PWD}/${RT_DIR}/${RUNTIME}"
exit $?
