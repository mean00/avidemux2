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
check_prerequisites()
{
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
        if [ -z "${SRCDIR}/${i}" ]; then
            echo "${i} not found in ${SRCDIR}"
            missing=1
        fi
    done

    [ ${missing} -ne 0 ] && fail "Requirements not fulfilled. Aborting"
}
apply_patch_from_location()
{
    local pbase=$1
    local ploc=$2
    if [ -n "$ploc" ]; then
        ploc="${ploc}/"
    fi
    patch --dry-run -d "${MXE_ROOT_DIR}" -p1 < "${ploc}${pbase}.patch" \
    && patch -d "${MXE_ROOT_DIR}" -p1 < "${ploc}${pbase}.patch" || fail "Failed at $pbase patch"
}
backout_patch()
{
    local pbase=$1
    local in="${SRCDIR}/backout/${pbase}.patch"
    patch --dry-run -d "${MXE_ROOT_DIR}" -Rp1 < "$in" \
    && patch -d "${MXE_ROOT_DIR}" -Rp1 < "$in" || fail "Failed at $pbase patch"
}
apply_patch_current()
{
    apply_patch_from_location $1 "$PWD"
}
apply_patch()
{
    apply_patch_from_location $1 "$SRCDIR"
}
prepare_sources()
{
    # check for collisions
    [ -e "${TOP}/mxe" ] && fail "mxe directory or file exists in the current directory, please rename or delete it. Aborting"

    # get the source
    git clone https://github.com/mxe/mxe.git || fail "Cannot clone MXE repository. Aborting"

    if [ "$use_last_known_good" -eq 1 ]; then
        if [ -f "${SRCDIR}/mxe-last-known-good" ]; then
            . "${SRCDIR}/mxe-last-known-good"
        else
            echo "Last known good MXE revision requested, but file \"${SRCDIR}/mxe-last-known-good\" not present."
            fail "No last known good"
        fi
        pushd "${TOP}/mxe" > /dev/null && \
        git checkout -b last-known-good-rev "${MXE_LAST_KNOWN_GOOD_REVISION}" || fail "Failed at checking out last known good revision"
        popd > /dev/null
    fi

    [ -d "${MXE_ROOT_DIR}/pkg" ] || mkdir "${MXE_ROOT_DIR}/pkg" || fail "Cannot create folder for packages"

    # get x264 source
    ("${SRCDIR}/${x264_script}") \
    && cp -uv x264-*.tar.bz2 "${MXE_ROOT_DIR}/pkg/" || fail "Failed at x264 source"
    # patch MXE
    apply_patch_current x264_gen

    # Patch MXE to download and build more recent versions of some other codecs:
    # fdk-aac 2.0.0 --> 2.0.3
    apply_patch fdk-aac

    # libvpx 1.8.2 --> 1.14.1
    apply_patch libvpx

    # opus 1.3.1 --> 1.5.2
    apply_patch opus

    # x265 3.4 --> 4.1
    apply_patch x265

    # download.qt.io redirects to the closest mirror, and at least one of the mirrors
    # hosts corrupted files, causing download to hang (which may be a MXE bug).
    # Hardcode a known good mirror.
    apply_patch qtbase-download-url
    apply_patch qt6-qtbase-download-url

    # MXE update of SQLite to 3.49.0 broke its installation, revert to 3.48.0
    #backout_patch sqlite-update-to-349000
    #backout_patch sqlite-correct-library-extension
}
build_mxe()
{
    #echo "MXE_TARGETS :=  i686-w64-mingw32.shared x86_64-w64-mingw32.shared" > "${MXE_ROOT_DIR}/settings.mk"
    echo "MXE_TARGETS :=  x86_64-w64-mingw32.shared" > "${MXE_ROOT_DIR}/settings.mk"

    # now build MXE
    pushd "${MXE_ROOT_DIR}" > /dev/null && MXE_SILENT_NO_NETWORK= \
    make \
    MXE_PLUGIN_DIRS="${MXE_ROOT_DIR}/plugins/gcc14" \
    $PACKAGES \
    $EXTRA_MXE_PACKAGES || fail "Failed at make"
    popd > /dev/null
}
install_libaom()
{
    # get and install libaom
    (AOM_TAG="v3.11.0" "${SRCDIR}/${libaom_script}" "${MXE_ROOT_DIR}") || fail "Failed at libaom"
}
install_nvidia_headers()
{
    # install nv-codec-headers for x86_64 only
    if [ ! -d "${MXE_ROOT_DIR}/usr/x86_64-w64-mingw32.shared" ]; then
        echo "Install prefix directory for 64-bit target doesn't exist, skipping."
    else
        if [ -d "${TOP}/nv-codec-headers" ]; then
            rm -rf "${TOP}/nv-codec-headers" || fail "Cannot remove existing nv-codec-headers source"
        fi

        git clone https://github.com/FFmpeg/nv-codec-headers.git || fail "Cannot clone nv-codec-headers repository"

        pushd nv-codec-headers > /dev/null \
        && git checkout sdk/11.1 \
        && sed -i '/PREFIX\ =\ \/usr\/local/d' Makefile \
        && PREFIX="${MXE_ROOT_DIR}/usr/x86_64-w64-mingw32.shared" make install || fail "Cannot install nv-codec-headers"
        popd > /dev/null
    fi
}
show_usage()
{
    echo "Usage: $(basename $0) [options]"
    echo
    echo "When executed without options, clone MXE git repository into current"
    echo "directory, get latest x264 code from the stable branch, patch MXE source,"
    echo "build MXE and packages, clone libaom source, build and install it in MXE"
    echo "target, clone nv-codec-headers and install it in MXE."
    echo
    echo "Options:"
    echo
    echo " --build          Just run make, skip cloning and patching the source."
    echo "                  This option disables automatic installation of libaom"
    echo "                  and nv-codec-headers, add --libaom and --nvidia-headers"
    echo "                  correspondingly to enable."
    echo
    echo " --help           Print usage and exit"
    echo
    echo " --libaom         Clone the source, build and install libaom"
    echo
    echo " --nv-headers     Clone the source and install nv-codec-headers"
    echo
    echo " --prepare        Clone MXE repository, get x264 source, patch MXE but"
    echo "                  do not perform build. --prepare takes precedence over"
    echo "                  all other options except of --help."
    echo
    echo " --last-good      When preparing sources, check out the last known working"
    echo "                  MXE revision."
}

TOP=$(pwd)
SRCDIR="$(dirname $(realpath $0))"
MXE_ROOT_DIR="${TOP}/mxe"

x264_script="x264-snapshot.sh"
libaom_script="install-libaom.bash"

PACKAGES="faad2 \
fdk-aac \
fribidi \
lame \
libass \
libvpx \
nsis \
ogg \
opus \
qttools \
qtwinextras \
qt6-qttools \
sdl2 \
vorbis \
x264 \
x265 \
xvidcore"

# default options
do_prepare_only=0
do_build_only=0
do_libaom_only=0
do_nv_codec_only=0
use_last_known_good=0

while [ $# != 0 ]; do
    opt="$1"
    case "$opt" in
        --help)
            show_usage
            exit 0
        ;;
        --prepare)
            do_prepare_only=1
        ;;
        --build)
            do_build_only=1
        ;;
        --libaom)
            do_libaom_only=1
        ;;
        --nv-headers)
            do_nv_codec_only=1
        ;;
        --last-good)
            use_last_known_good=1
        ;;
        *)
            echo "Unknown option $opt"
            show_usage
            exit 1
        ;;
    esac
    shift
done

check_prerequisites
if [ "$do_build_only" -eq 0 -a "$do_libaom_only" -eq 0 -a "$do_nv_codec_only" -eq 0 ] || [ "$do_prepare_only" -eq 1 ]; then
    prepare_sources
fi
if [ "$do_prepare_only" -eq 0 ]; then
    if [ "$do_build_only" -eq 1 ] || [ "$do_libaom_only" -eq 0 -a "$do_nv_codec_only" -eq 0 ]; then
        build_mxe
    fi
    if [ "$do_libaom_only" -eq 1 ] || [ "$do_build_only" -eq 0 -a "$do_nv_codec_only" -eq 0 ]; then
        install_libaom
    fi
    if [ "$do_nv_codec_only" -eq 1 ] || [ "$do_build_only" -eq 0 -a "$do_libaom_only" -eq 0 ]; then
        install_nvidia_headers
    fi
fi

echo "All done"
exit 0
