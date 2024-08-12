#!/bin/bash
fail()
{
    echo "** $1 **"
    exit 1
}
check()
{
    to_check=$1
    which "${to_check}" > /dev/null 2>&1 || fail "\"${to_check}\" not found in PATH. Aborting"
}

cur=$(pwd)
where="$(dirname $(realpath $0))"
x264_script="x264-snapshot.sh"
libaom_script="install-libaom.bash"
declare -a reqs
reqs+=(${x264_script})
reqs+=(${libaom_script})

# check prerequisites
check patch
check sed
check sha256sum
check git
git rev-parse --show-toplevel > /dev/null 2>&1 && fail "Do not run this script from within a git repository. Aborting"

missing=0

for i in "${reqs[@]}"; do
    if [ -z "${where}/${i}" ]; then
        echo "${i} not found in ${where}"
        missing=1
    fi
done

[ ${missing} -ne 0 ] && fail "Requirements not fulfilled. Aborting"

# check for collisions
[ -e "${cur}/mxe" ] && fail "mxe directory or file exists in the current directory, please rename or delete it. Aborting"

# get the source
git clone https://github.com/mxe/mxe.git || fail "Cannot clone MXE repository. Aborting"

MXE_ROOT_DIR="${cur}/mxe"

[ -d "${MXE_ROOT_DIR}/pkg" ] || mkdir "${MXE_ROOT_DIR}/pkg" || fail "Cannot create folder for packages"

# get x264 source
("${where}/${x264_script}") || fail "Failed at x264 source"
# patch MXE
patch --dry-run -d "${MXE_ROOT_DIR}" -p1 < x264_gen.patch \
&& patch -d "${MXE_ROOT_DIR}" -p1 < x264_gen.patch \
&& cp -uv x264-*.tar.bz2 "${MXE_ROOT_DIR}/pkg/" || fail "Failed at x264 patch"

# Patch MXE to download and build more recent versions of some other codecs:
# fdk-aac 2.0.0 --> 2.0.3
patch --dry-run -d "${MXE_ROOT_DIR}" -p1 < "${where}/fdk-aac.patch" \
&& patch -d "${MXE_ROOT_DIR}" -p1 < "${where}/fdk-aac.patch" || fail "Failed at fdk-aac patch"

# libvpx 1.8.2 --> 1.13.1
patch --dry-run -d "${MXE_ROOT_DIR}" -p1 < "${where}/libvpx.patch" \
&& patch -d "${MXE_ROOT_DIR}" -p1 < "${where}/libvpx.patch" || fail "Failed at libvpx patch"

# opus 1.3.1 --> 1.5.2
patch --dry-run -d "${MXE_ROOT_DIR}" -p1 < "${where}/opus.patch" \
&& patch -d "${MXE_ROOT_DIR}" -p1 < "${where}/opus.patch" || fail "Failed at opus patch"

# x265 3.4 --> 3.6
patch --dry-run -d "${MXE_ROOT_DIR}" -p1 < "${where}/x265.patch" \
&& patch -d "${MXE_ROOT_DIR}" -p1 < "${where}/x265.patch" || fail "Failed at x265 patch"

#echo "MXE_TARGETS :=  i686-w64-mingw32.shared x86_64-w64-mingw32.shared" > "${MXE_ROOT_DIR}/settings.mk"
echo "MXE_TARGETS :=  x86_64-w64-mingw32.shared" > "${MXE_ROOT_DIR}/settings.mk"

# now build MXE
cd "${MXE_ROOT_DIR}" && MXE_SILENT_NO_NETWORK= \
make \
MXE_PLUGIN_DIRS="${MXE_ROOT_DIR}/plugins/gcc14" \
faad2 \
fdk-aac \
fribidi \
lame \
libass \
libvpx \
ogg \
opus \
qttools \
qtwinextras \
sdl2 \
vorbis \
x264 \
x265 \
xvidcore || fail "Failed at make"

cd "${cur}"

# get and install libaom
(AOM_TAG="v3.9.1" "${where}/${libaom_script}" "${MXE_ROOT_DIR}") || fail "Failed at libaom"

if [ -z "${MXE_ROOT_DIR}/usr/x86_64-w64-mingw32.shared" ]; then
    echo "All done" && exit 0
fi

# install nv-codec-headers for x86_64 only
if [ -d "${cur}/nv-codec-headers" ]; then
    rm -rf "${cur}/nv-codec-headers" || fail "Cannot remove existing nv-codec-headers source"
fi

git clone https://github.com/FFmpeg/nv-codec-headers.git || fail "Cannot clone nv-codec-headers repository"
cd nv-codec-headers \
&& git checkout sdk/11.1 \
&& sed -i '/PREFIX\ =\ \/usr\/local/d' Makefile \
&& PREFIX="${MXE_ROOT_DIR}/usr/x86_64-w64-mingw32.shared" make install || fail "Cannot install nv-codec-headers"

# the end
cd "${cur}"
echo "All done"
exit 0
