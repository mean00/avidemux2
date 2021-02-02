#!/bin/bash
# This is a convenience script to automate installation of libaom in MXE
failMsg()
{
    echo "** $1 **"
    exit 1
}
process()
{
    case "$1" in
    32)
        CROSS_PREFIX="i686-w64-mingw32.shared"
        TOOLCHAIN="x86-mingw-gcc.cmake"
        ;;
    64)
        CROSS_PREFIX="x86_64-w64-mingw32.shared"
        TOOLCHAIN="x86_64-mingw-gcc.cmake"
        ;;
    *)
        echo "Unsupported option \"$1\""
        exit 1
        ;;
    esac
    if [ -d "build-aom$1" ]; then
        rm -rf "build-aom$1" || failMsg "Cannot remove old ${AOM_HOME}/build-aom$1 directory"
    fi
    mkdir "build-aom$1" || failMsg "Cannot create ${AOM_HOME}/build-aom$1 directory"
    cd "build-aom$1"

    cmake ../aom/ \
        -DCROSS="${CROSS_PREFIX}-" \
        -DCMAKE_TOOLCHAIN_FILE="../aom/build/cmake/toolchains/${TOOLCHAIN}" \
        -DENABLE_DOCS=0 \
        -DENABLE_EXAMPLES=0 \
        -DENABLE_TOOLS=0 \
        -DBUILD_SHARED_LIBS=1 \
        -DCONFIG_AV1_ENCODER=0 \
        -DCONFIG_ANALYZER=0 \
        -DFORCE_HIGHBITDEPTH_DECODING=0 \
        -DCMAKE_INSTALL_PREFIX=${MXE_ROOT}/usr/${CROSS_PREFIX} || failMsg "Failed at cmake"
    make -j $(nproc) || failMsg "Failed at make"
    make install || failMsg "Failed at make install"
    DLL="${MXE_ROOT}/usr/${CROSS_PREFIX}/lib/libaom.dll"
    mv -v "${DLL}" "${MXE_ROOT}/usr/${CROSS_PREFIX}/bin/" || failMsg "Failed moving ${DLL} to ${MXE_ROOT}/usr/${CROSS_PREFIX}/bin/"
}

#AOM_HOME=/home/eumagga/adm
#MXE_ROOT=${AOM_HOME}/mxe

MXE_ROOT="$1"

if [ "x${MXE_ROOT}" = "x" ]; then
    echo "Usage: $0 /path/to/MXE/root/directory"
    exit 1
fi
if ! [ -d "${MXE_ROOT}" ]; then
    failMsg "${MXE_ROOT} does not exist or is not a directory"
fi
if ! [ -f "${MXE_ROOT}/mxe.github.mk" ]; then
    failMsg "${MXE_ROOT} does look like a MXE root directory"
fi

BUILD_32_BITS=0
BUILD_64_BITS=0

if [ -d "${MXE_ROOT}/usr/i686-w64-mingw32.shared" ]; then
    BUILD_32_BITS=1
fi

if [ -d "${MXE_ROOT}/usr/x86_64-w64-mingw32.shared" ]; then
    BUILD_64_BITS=1
fi

if [ ${BUILD_32_BITS} -eq 0 ] && [ ${BUILD_64_BITS} -eq 0 ]; then
    failMsg "No supported build targets found"
fi

if [ "x${AOM_HOME}" = "x" ]; then
    export AOM_HOME="$(pwd)"
fi
if ! [ -d "${AOM_HOME}" ]; then
    failMsg "${AOM_HOME} does not exist or is not a directory"
fi
cd "${AOM_HOME}"
if [ -d "${AOM_HOME}/aom" ] && ! [ -d "${AOM_HOME}/aom/.git" ]; then
    echo "${AOM_HOME}/aom is not a git repository, will try to remove it"
    rm -rf "${AOM_HOME}/aom" || failMsg "Cannot remove ${AOM_HOME}/aom"
fi

FRESH=0

if ! [ -d "${AOM_HOME}/aom" ]; then
    echo "Will clone aom source in ${AOM_HOME} directory"
    git clone https://aomedia.googlesource.com/aom || failMsg "Failed cloning aom source"
    FRESH=1
fi
cd "${AOM_HOME}/aom"

if [ "x${AOM_TAG}" = "x" ]; then
    AOM_TAG="v2.0.1"
fi

if [ "x${FRESH}" = "x0" ]; then
    git fetch || failMsg "Cannot fetch changes"
fi
git checkout tags/${AOM_TAG} || failMsg "Cannot checkout ${AOM_TAG}"

cd ..

export PATH="${PATH}:${MXE_ROOT}/usr/bin"

RESULT=0

if [ ${BUILD_32_BITS} -eq 1 ]; then
    process 32 || ( echo "32-bit build failed"; RESULT=1 )
fi
if [ ${BUILD_64_BITS} -eq 1 ]; then
    process 64 || ( echo "64-bit build failed"; RESULT=1 )
fi
if [ $RESULT -eq 1 ]; then
    failMsg "Some builds failed"
fi
echo "All done"
exit 0
