#!/bin/bash
# This is a convenience script to automate installation of libaom in MXE
failMsg()
{
    echo "** $1 **"
    exit 1
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

CROSS_PREFIX=x86_64-w64-mingw32.shared

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
if ! [ -d "${AOM_HOME}/aom" ]; then
    echo "Will clone aom source in ${AOM_HOME} directory"
    git clone https://aomedia.googlesource.com/aom || failMsg "Failed cloning aom source"
fi
cd "${AOM_HOME}/aom"
git checkout tags/v2.0.0 || failMsg "Cannot checkout v2.0.0"

cd ..
if [ -d build-aom ]; then
    rm -rf build-aom || failMsg "Cannot remove old ${AOM_HOME}/build-aom directory"
fi
mkdir build-aom || failMsg "Cannot create ${AOM_HOME}/build-aom directory"
cd build-aom

export PATH=${PATH}:${MXE_ROOT}/usr/bin

cmake ../aom/ \
    -DCROSS="${CROSS_PREFIX}-" \
    -DCMAKE_TOOLCHAIN_FILE=../aom/build/cmake/toolchains/x86_64-mingw-gcc.cmake \
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
echo "All done"
exit 0
