#!/bin/bash
nodeps=""
nofail=""
nobuild=""
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
    echo "  --no-install-deps : Do not install missing dependencies, fail instead"
    echo "  --no-fail-missing : Try to continue even if dependencies are missing"
    echo "  --rebuild         : Preserve existing build directories"
}
#
fail()
{
    echo "$@"
    exit 1
}
#
check_nvenc()
{
    if (pkg-config --exists ffnvcodec); then
        return 0
    elif [ "x${nodeps}" = "x1" ]; then
        echo "NVENC headers not found and installation disabled on command line."
        return 1
    fi
    echo "nv-codec-headers are missing, will try to install."
    rm -rf /tmp/nvenc > /dev/null 2>&1
    mkdir /tmp/nvenc || return 1
    cd /tmp/nvenc || return 1
    git clone https://github.com/FFmpeg/nv-codec-headers.git || return 1
    cd nv-codec-headers || return 1
    # We need at least 11.1
    git checkout sdk/11.1 || return 1
    make || return 1
    sudo make install || return 1
}
#
check_aom()
{
    if (pkg-config --exists aom); then
        echo "aom is present, checking version..."
        AOM_VERSION=$(pkg-config --modversion aom)
        AOM_VERSION_MAJOR=$(echo "${AOM_VERSION}" | cut -d \. -f 1 - )
        AOM_VERSION_MINOR=$(echo "${AOM_VERSION}" | cut -d \. -f 2 - )
        if [ "$AOM_VERSION_MAJOR" -gt "3" ] || { [ "$AOM_VERSION_MAJOR" -eq "3" ] && [ $AOM_VERSION_MINOR -ge "6" ]; }; then
            echo "aom version ${AOM_VERSION} is sufficient."
            return 0
        elif [ "x${nodeps}" = "x1" ]; then
            echo "aom version is too old and installation disabled on command line."
            return 1
        fi
    elif [ "x${nodeps}" = "x1" ]; then
        echo "aom not found and installation disabled on command line."
        return 1
    fi
    echo "Minimum required version of aom is missing, will try to install."
    if ! [ -d "aom" ]
    then
        echo "Will clone aom source to current directory"
        git clone https://aomedia.googlesource.com/aom || return 1
    else
        echo "Trying to re-use existing aom source directory"
        git fetch || fail "Cannot fetch changes"
    fi
    pushd "aom" > /dev/null && git checkout tags/v3.9.1 || return 1
    popd > /dev/null
    if [ -d "build-aom" ]
    then
        rm -rf "build-aom" || return 1
    fi
    mkdir "build-aom" || return 1
    pushd "build-aom" > /dev/null || return 1
    cmake ../aom/ \
    -DENABLE_DOCS=0 \
    -DENABLE_EXAMPLES=0 \
    -DENABLE_TOOLS=0 \
    -DBUILD_SHARED_LIBS=1 \
    -DCONFIG_ANALYZER=0 \
    -DFORCE_HIGHBITDEPTH_DECODING=0 \
    -DCMAKE_INSTALL_PREFIX="/usr/local" || return 1
    make -j $(nproc) || return 1
    sudo make install || return 1
    popd > /dev/null
    return 0
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
        if [ "x${nodeps}" != "x1" ]; then
            echo "Warning, non-free repositories must be already enabled on this system to install them."
        fi
    fi
    nb_missing=$(( ${#missing_required[@]} + ${#missing_nonfree[@]} ))
    if [ ${nb_missing} -gt 0 ]; then
        if [ "x${nodeps}" = "x1" ]; then
            fail "Installation of build dependencies disabled on command line, aborting."
        elif [ "x${nofail}" = "x1" ]; then
            echo "Trying to continue nevertheless"
        else
            sudo /usr/bin/apt-get update || fail "Cannot sync repo metadata"
            sudo /usr/bin/apt-get install ${missing_required[*]} ${missing_nonfree[*]} || fail "Failed to install all build dependencies, aborting."
        fi
    fi
    if (check_nvenc); then
        echo "NVENC headers found."
    elif [ "x${nofail}" = "x1" ]; then
        echo "Cannot install NVENC headers, trying to continue nevertheless."
    else
        fail "Cannot install NVENC headers."
    fi
    if (check_aom); then
        echo "libaom >= 3.6.0 found"
    elif [ "x${nofail}" = "x1" ]; then
        echo "Cannot install required version of libaom, trying to continue nevertheless."
    else
        fail "Cannot install required version of libaom."
    fi
}
#
echo "Automatic AppImage generator for Avidemux, Debian 12.6 version"
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
            nobuild="1"
            ;;
        --no-install-deps)
            nodeps="1"
            ;;
        --no-fail-missing)
            nofail="1"
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

if [ "x${nobuild}" = "x1" ]; then
    exit $?
fi

RUNTIME="runtime-x86_64"
RT_DIR="externalBinaries/AppImageKit"
SHA256SUM="24da8e0e149b7211cbfb00a545189a1101cb18d1f27d4cfc1895837d2c30bc30" # size: 188392 bytes
TO_CHECK=""
DO_DOWNLOAD=0
echo "Current directory: \"${PWD}\""
if [ ! -e "${RT_DIR}" ]; then
    mkdir -p "${RT_DIR}" || exit 1
fi
pushd "${RT_DIR}" > /dev/null || exit 1
if [ -f "${RUNTIME}" ]; then
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
popd > /dev/null
if [ "${SHA256SUM}" != "${TO_CHECK}" ]; then
    echo "Checksum doesn't match, aborting."
    exit 1
fi

rm -rf install > /dev/null 2>&1

SRCTOP=$(cd $(dirname "$0") && pwd)
logfile="/tmp/log-bootstrap-$(date +%F_%T).log"
bash "${SRCTOP}/bootStrap.bash" --with-system-libmad ${rebuild} 2>&1 | tee ${logfile}
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    fail "Build failed, please inspect ${logfile} and /tmp/logbuild* files."
fi
bash "${SRCTOP}/appImage/deployBookwormMinimal.sh" "${PWD}/${RT_DIR}/${RUNTIME}"
exit $?
