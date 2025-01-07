#!/bin/bash

# Default config

bits=64
default_mxerootdir="/opt/mxe"
mxerootdir="$default_mxerootdir"
rebuild=0
debug=0
do_core=1
do_qt=1
do_cli=1
do_plugins=1
external_liba52=0
external_libmad=0
do_release_pkg=1
author_setup=0

# Functions

authorSetup()
{
    export SDLDIR=/mingw
    export MINGW=/mingw
    export MINGWDEV=/mingw_dev
    export PATH=${MINGW}/bin:$PATH
    export INSTALL_DIR=${MINGW}/Release
    export QT_HOME=/mingw/Qt/current
    export O_PARAL="-j 2"
    export TOOLCHAIN_LOCATION=/mingw
    export CFLAGS="-I/mingw/include -L/mingw/lib"
}

setupEnv()
{
    export BUILDDATE=$(date +%y%m%d-%H%M%S)
    if [ "x$external_liba52" = "x1" ]; then
        export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBA52=true $EXTRA_CMAKE_DEFS"
    fi
    if [ "x$external_libmad" = "x1" ]; then
        export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBMAD=true $EXTRA_CMAKE_DEFS"
    fi
    export BUILDTOP=$PWD
    if [[ $BUILDTOP = *" "* ]]; then
        echo "The build directory path \"${BUILDTOP}\" contains one or more spaces."
        echo "This is unsupported by FFmpeg configure."
        fail "build prerequisites"
    fi
    export SRCTOP=$(cd $(dirname "$0") && pwd)
    export BITS=$bits
    if [ "x$BITS" = "x64" ]; then
        export ARCH="x86_64"
    else
        export ARCH="i686"
    fi
    export MXE_ROOT="$mxerootdir"
    export MXE_TARGET=${ARCH}-w64-mingw32.shared
    export QT_SELECT=5
    if [ "x$author_setup" = "x1" ]; then
        authorSetup
    else
        export MINGW="${MXE_ROOT}/usr/${MXE_TARGET}"
        export QT_HOME="${MINGW}"/qt5
        export PATH="$PATH":"${MXE_ROOT}"/usr/bin:"${QT_HOME}"/bin
        export TOOLCHAIN_LOCATION="${MXE_ROOT}"/usr
        export SDL2DIR="$MINGW"
        PARAL="-j $(nproc)"
        if [ "x$debug" != "x1" ]; then
            export INSTALL_DIR="${MINGW}"/out/avidemux
        else
            export INSTALL_DIR="${MINGW}"/out_debug/avidemux
        fi
    fi
    export CROSS_PREFIX=$MXE_TARGET
    export PKG_CONFIG_PATH="${MINGW}"/lib/pkgconfig
    export PKG_CONFIG_LIBDIR="${MINGW}"/lib/pkgconfig
    export CXXFLAGS="-std=c++11"
    export CROSS_C_COMPILER=gcc
    export CROSS_CXX_COMPILER=g++
}

fail()
{
    echo "** Failed at $1 **"
    exit 1
}

Process()
{
    BASE=$1
    SOURCEDIR=$2
    EXTRA=$3
    GENERATOR="Unix Makefiles"
    if [ "x$debug" = "x1" ]; then
                DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug"
                BASE="${BASE}_debug"
                GENERATOR="CodeBlocks - Unix Makefiles"
    fi
    BUILDDIR="${PWD}/${BASE}"
    echo "Building in \"${BUILDDIR}\" from \"${SOURCEDIR}\" with EXTRA=<${EXTRA}>)"
    if [ "x$rebuild" != "x1" ]; then
        rm -Rf "$BUILDDIR"
    fi
    if [ ! -e "$BUILDDIR" ]; then
        mkdir "$BUILDDIR" || fail "creating build directory"
    fi
    pushd "$BUILDDIR" > /dev/null
    cmake -DCROSS="$MINGW" \
    -DCMAKE_SYSTEM_NAME:STRING=Windows \
    -DCMAKE_FIND_ROOT_PATH="$MINGW" \
    -DTOOLCHAIN_LOCATION="$TOOLCHAIN_LOCATION" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_C_COMPILER:STRING="${CROSS_PREFIX}-${CROSS_C_COMPILER}" \
    -DCMAKE_CXX_COMPILER:STRING="${CROSS_PREFIX}-${CROSS_CXX_COMPILER}" \
    -DCMAKE_LINKER:STRING="${CROSS_PREFIX}-ld" \
    -DCMAKE_AR:STRING="${CROSS_PREFIX}-ar" \
    -DCMAKE_RC_COMPILER:STRING="${CROSS_PREFIX}-windres" \
    -DAVIDEMUX_TOP_SOURCE_DIR:STRING="$SRCTOP" \
    ${DEBUG} \
    -G "${GENERATOR}" \
    $EXTRA \
    "$SOURCEDIR" || fail "cmake"
    make $PARAL >& /tmp/log$BASE || fail "make, result in /tmp/log$BASE"
    make install || fail "install"
    # Only install  component=dev for dev package
    if [ "x$author_setup" = "x1" ]; then
        DESTDIR=${MINGWDEV} cmake -DCOMPONENT=dev -P cmake_install.cmake || fail make_install_dev
    fi
    popd > /dev/null
}

usage()
{
    echo "Usage: bash $0 [OPTION]"
    echo "  --help                 : Print usage"
    echo "  --32                   : Build a 32 bit application"
    echo "  --64                   : Build a 64 bit application (default)"
    echo "  --debug                : Switch debugging on"
    echo "  --mxe-root=DIR         : Use MXE installed in DIR (default: ${default_mxerootdir})"
    echo "  --rebuild              : Preserve existing build directories"
    echo "  --with-core            : Build core (default)"
    echo "  --without-core         : Don't build core"
    echo "  --with-cli             : Build cli (default)"
    echo "  --without-cli          : Don't build cli application and plugins"
    echo "  --with-qt              : Build Qt (default)"
    echo "  --without-qt           : Don't build Qt application and plugins"
    echo "  --with-plugins         : Build plugins (default)"
    echo "  --without-plugins      : Don't build plugins"
    echo "  --with-system-liba52   : Use the system liba52 (a52dec) instead of the bundled one"
    echo "  --with-system-libmad   : Use the system libmad instead of the bundled one"
    echo "  --nopkg                : Don't create a ZIP archive with all required libraries"
    echo "  -a, --author           : Match the env setup used by the Author, implies --nopkg"
}

option_value()
{
    echo $(echo $* | cut -d '=' -f 2-)
}

option_name()
{
    echo $(echo $* | cut -d '=' -f 1 | cut -b 3-)
}

dir_check()
{
    op_name="$1"
    dir_path="$2"
    if [ "x$dir_path" != "x" ] ; then
        if [[ "$dir_path" != /* ]] ; then
            >&2 echo "Expected an absolute path for --$op_name=$dir_path, aborting."
            exit 1
        fi
    else
        >&2 echo "Empty path provided for --$op_name, aborting."
        exit 1
    fi
    case "$dir_path" in
      */)
          echo $(expr "x$dir_path" : 'x\(.*[^/]\)') # strip trailing slashes
          ;;
      *)
          echo "$dir_path"
          ;;
    esac
}

create_release_package()
{
    if [ ! -e "${INSTALL_DIR}"/avidemux.exe ]; then
        echo "No avidemux.exe (${BITS} bit) found in ${INSTALL_DIR}, aborting"
        exit 1
    fi
    echo "Preparing package..."
    pushd "$BUILDTOP" > /dev/null
    PACKAGE_DIR="packaged_mingw_build_${BUILDDATE}"
    if [ "x$debug" = "x1" ]; then
        PACKAGE_DIR="packaged_mingw_debug_build_${BUILDDATE}"
    fi
    PACKAGE_DIR="${BUILDTOP}/${PACKAGE_DIR}"
    if [ ! -e "$PACKAGE_DIR" ]; then
        mkdir "$PACKAGE_DIR" || fail "creating package directory"
    fi
    cp -a "$INSTALL_DIR" "$PACKAGE_DIR"
    cd "$PACKAGE_DIR"
    mv -v avidemux avidemux_$BITS
    TARGETDIR="${PACKAGE_DIR}/avidemux_$BITS"
    if [ ! -e "${TARGETDIR}"/platforms ]; then
        mkdir "${TARGETDIR}"/platforms || fail "creating platforms directory"
    fi
    if [ ! -e "${TARGETDIR}"/styles ]; then
        mkdir "${TARGETDIR}"/styles || fail "creating styles directory"
    fi
    cd "${MINGW}"/bin
    if [ "x${external_liba52}" = "x1" ]; then
        cp -v liba52-*.dll "$TARGETDIR"
    fi
    if [ "x${external_libmad}" = "x1" ]; then
        cp -v libmad-*.dll "$TARGETDIR"
    fi
    cp -v \
    libaom.dll \
    libass-*.dll \
    libbrotlicommon.dll \
    libbrotlidec.dll \
    libbz2.dll \
    libcrypto-*.dll \
    libexpat-*.dll \
    libfaad-*.dll \
    libfdk-aac-*.dll \
    libffi-*.dll \
    libfontconfig-*.dll \
    libfreetype-*.dll \
    libfribidi-*.dll \
    libgcc_*.dll \
    libglib-*.dll \
    libgobject-*.dll \
    libharfbuzz-0.dll \
    libiconv-*.dll \
    icudt*.dll \
    icuin*.dll \
    icuuc*.dll \
    libintl-*.dll \
    libmp3lame-*.dll \
    libogg-*.dll \
    libopus-*.dll \
    libpcre2-16-*.dll \
    libpcre2-8-*.dll \
    libpng16-*.dll \
    libsamplerate-*.dll \
    libsqlite3-*.dll \
    libssl-*.dll \
    libssp-*.dll \
    libstdc++-*.dll \
    libvorbis-*.dll \
    libvorbisenc-*.dll \
    libvorbisfile-*.dll \
    libwinpthread-*.dll \
    libx264-*.dll \
    libx265.dll \
    libzstd.dll \
    SDL2.dll \
    xvidcore.dll \
    zlib1.dll \
    "$TARGETDIR";
    cd "$QT_HOME"
    cp -v \
    bin/Qt5Core.dll \
    bin/Qt5Gui.dll \
    bin/Qt5Network.dll \
    bin/Qt5Widgets.dll \
    bin/Qt5WinExtras.dll \
    "$TARGETDIR";
    cp -v \
    plugins/platforms/qminimal.dll \
    plugins/platforms/qwindows.dll \
    "${TARGETDIR}"/platforms/;
    cp -v \
    plugins/styles/qwindowsvistastyle.dll \
    "${TARGETDIR}"/styles/;
    mkdir "${TARGETDIR}"/etc || fail "creating etc directory"
    cp -rvL "${MINGW}"/etc/fonts "${TARGETDIR}"/etc
    cd "$TARGETDIR"
    if [ ! "x$debug" = "x1" ]; then
        find . -name "*.dll.a" -exec rm -v '{}' \;
        rm -Rf include
    fi
    cd "$PACKAGE_DIR"
    zip -r avidemux_r${BUILDDATE}_win${BITS}Qt5.zip avidemux_$BITS
    rm -Rf avidemux_$BITS
    popd > /dev/null
    echo "Avidemux Windows package generated as \"${PACKAGE_DIR}/avidemux_r${BUILDDATE}_win${BITS}Qt5.zip\""
}

# Options handling

while [ $# != 0 ]; do
    config_option="$1"
    case "$config_option" in
        -h|--help)
            usage
            exit 1
            ;;
        --32)
            bits=32
            ;;
        --64)
            bits=64
            ;;
        -a|--author)
            export author_setup=1
            do_release_pkg=0
            ;;
        --mxe-root=*)
            mxerootdir=$(dir_check $(option_name "$config_option") $(option_value "$config_option")) || exit 1
            ;;
        --debug)
            debug=1
            ;;
        --rebuild)
            rebuild=1
            ;;
        --without-qt)
            do_qt=0
            ;;
        --without-cli)
            do_cli=0
            ;;
        --without-plugins)
            do_plugins=0
            ;;
        --without-core)
            do_core=0
            ;;
        --with-qt)
            do_qt=1
            ;;
        --with-cli)
            do_cli=1
            ;;
        --with-plugins)
            do_plugins=1
            ;;
        --with-core)
            do_core=1
            ;;
        --with-system-liba52)
            export external_liba52=1
            ;;
        --with-system-libmad)
            export external_libmad=1
            ;;
        --nopkg)
            do_release_pkg=0
            ;;
        *)
            echo "unknown parameter $config_option"
            usage
            exit 1
            ;;
        esac
    shift
done


# Set all the required paths and env variables

setupEnv

# Create destination directory

echo "** Bootstrapping Avidemux **"
if [ "x$author_setup" = "x1" ]; then
    rm -Rf "${MINGWDEV}"/*
fi
if [ -e "$INSTALL_DIR" -a "x$do_core" = "x1" -a "x$do_qt" = "x1" ]; then
    rm -Rf "$INSTALL_DIR";
fi
mkdir -p "$INSTALL_DIR"
echo "Build top dir : $BUILDTOP"

# Build and install Avidemux components to the destination dir

if [ "x$do_core" = "x1" ]; then 
    echo "** CORE **"
    Process buildMingwCore-${ARCH} "${SRCTOP}"/avidemux_core "-DCMAKE_CROSS_PREFIX=${CROSS_PREFIX}"
fi

if [ "x$do_qt" = "x1" ]; then
    echo "** QT **"
    Process buildMingwQt5-${ARCH} "${SRCTOP}"/avidemux/qt4 "-DQT_HOME:STRING=${QT_HOME} -DENABLE_QT5=true"
fi

if [ "x$do_cli" = "x1" ]; then
    echo "** CLI **"
    Process buildMingwCli-${ARCH} "${SRCTOP}"/avidemux/cli
fi

if [ "x$do_plugins" = "x1" ]; then
    echo "** Plugins **"
    Process buildMingwPluginsCommon-${ARCH} "${SRCTOP}"/avidemux_plugins "-DPLUGIN_UI=COMMON $EXTRA_CMAKE_DEFS"
fi

if [ "x$do_plugins" = "x1" -a "x$do_qt" = "x1" ]; then
    echo "** Plugins Qt **"
    Process buildMingwPluginsQt5-${ARCH} "${SRCTOP}"/avidemux_plugins "-DPLUGIN_UI=QT4 -DQT_HOME:STRING=$QT_HOME -DENABLE_QT5=true $EXTRA_CMAKE_DEFS"
fi

if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ]; then
    echo "** Plugins CLI **"
    Process buildMingwPluginsCli-${ARCH} "${SRCTOP}"/avidemux_plugins "-DPLUGIN_UI=CLI $EXTRA_CMAKE_DEFS"
fi

if [ "x$do_plugins" = "x1" ]; then
    echo "** Plugins Settings **"
    Process buildMingwPluginsSettings-${ARCH} "${SRCTOP}"/avidemux_plugins "-DPLUGIN_UI=SETTINGS $EXTRA_CMAKE_DEFS"
fi

# Create a ZIP archive with avidemux and all the required libs

if [ "x$do_release_pkg" = "x1" ]; then
    create_release_package
fi

echo "** All done **"
