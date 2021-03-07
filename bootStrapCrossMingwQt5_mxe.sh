#!/bin/bash

# Default config

bits=32
default_mxerootdir="/opt/mxe"
mxerootdir="$default_mxerootdir"
rebuild=0
debug=0
do_core=1
do_qt=1
do_cli=1
do_plugins=1
external_libass=1
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
    if [ "x$external_libass" = "x1" ]; then
        export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBASS=true $EXTRA_CMAKE_DEFS"
    fi
    if [ "x$external_liba52" = "x1" ]; then
        export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBA52=true $EXTRA_CMAKE_DEFS"
    fi
    if [ "x$external_libmad" = "x1" ]; then
        export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBMAD=true $EXTRA_CMAKE_DEFS"
    fi
    export TOP=$PWD
    export BITS=$bits
    if [ "x$BITS" = "x64" ]; then
        export ARCH="x86_64"
    else
        export ARCH="i686"
    fi
    export MXE_ROOT=$mxerootdir
    export MXE_TARGET=${ARCH}-w64-mingw32.shared
    export QT_SELECT=5
    if [ "x$author_setup" = "x1" ]; then
        authorSetup
    else
        export QT_HOME=${MXE_ROOT}/usr/${MXE_TARGET}/qt5
        export MINGW=${MXE_ROOT}/usr/${MXE_TARGET};
        export PATH=$PATH:${MXE_ROOT}/usr/bin:${QT_HOME}/bin
        export TOOLCHAIN_LOCATION=${MXE_ROOT}/usr
        export SDL2DIR=${MXE_ROOT}/usr/${MXE_TARGET}
        export PARAL="-j $(nproc)"
        if [ "x$debug" != "x1" ]; then
            export INSTALL_DIR=${MINGW}/out/avidemux
        else
            export INSTALL_DIR=${MINGW}/out_debug/avidemux
        fi
    fi
    export CROSS_PREFIX=$MXE_TARGET
    export PKG_CONFIG_PATH=$MINGW/lib/pkgconfig
    export PKG_CONFIG_LIBDIR=$MINGW/lib/pkgconfig
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
    cd $TOP
    export BUILDDIR=$1
    export SOURCEDIR=$2
    export EXTRA=$3
    GENERATOR="Unix Makefiles"
    if [ "x$debug" = "x1" ]; then
                DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug"
                BUILDDIR="${BUILDDIR}_debug"
                GENERATOR="CodeBlocks - Unix Makefiles"
    fi
    echo "Building $BUILDDIR from $SOURCEDIR (Extra=$EXTRA)"
    if [ "x$rebuild" != "x1" ]; then
        rm -Rf ./"$BUILDDIR"
    fi
    if [ ! -e "$BUILDDIR" ]; then
        mkdir "$BUILDDIR" || fail mkdir
    fi
    cd $BUILDDIR
    cmake -DCROSS=$MINGW \
    -DCMAKE_SYSTEM_NAME:STRING=Windows \
    -DCMAKE_FIND_ROOT_PATH=$MINGW \
    -DTOOLCHAIN_LOCATION=$TOOLCHAIN_LOCATION \
    -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
    -DCMAKE_CROSS_PREFIX=$CROSS_PREFIX \
    -DCMAKE_C_COMPILER:STRING=${CROSS_PREFIX}-${CROSS_C_COMPILER} \
    -DCMAKE_CXX_COMPILER:STRING=${CROSS_PREFIX}-${CROSS_CXX_COMPILER} \
    -DCMAKE_LINKER:STRING=${CROSS_PREFIX}-ld \
    -DCMAKE_AR:STRING=${CROSS_PREFIX}-ar \
    -DCMAKE_RC_COMPILER:STRING=${CROSS_PREFIX}-windres \
    -DAVIDEMUX_TOP_SOURCE_DIR="$TOP" \
    ${DEBUG} \
    -G "${GENERATOR}" \
    $EXTRA \
    $SOURCEDIR || fail cmake
    make $PARAL >& /tmp/log$BUILDDIR || fail "make, result in /tmp/log$BUILDDIR "
    make install || fail make_install
    # Only install  component=dev for dev package
    if [ "x$author_setup" = "x1" ]; then
        DESTDIR=${MINGWDEV} cmake -DCOMPONENT=dev -P cmake_install.cmake || fail make_install_dev
    fi
}

usage()
{
    echo "Usage: bash $0 [OPTION]"
    echo "  --help                 : Print usage"
    echo "  --32                   : Build a 32 bit application (default)"
    echo "  --64                   : Build a 64 bit application"
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
    echo "  --with-internal-libass : Use bundled libass instead of the system one"
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
    if [ ! -e ${INSTALL_DIR}/avidemux.exe ]; then
        echo "No avidemux.exe (${BITS} bit) found in ${INSTALL_DIR}, aborting"
        exit 1
    fi
    echo "Preparing package..."
    cd $TOP
    PACKAGE_DIR="packaged_mingw_build_${BUILDDATE}"
    if [ "x$debug" = "x1" ]; then
        PACKAGE_DIR="packaged_mingw_debug_build_${BUILDDATE}"
    fi
    if [ ! -e $PACKAGE_DIR ]; then
        mkdir $PACKAGE_DIR
    fi
    cp -a $INSTALL_DIR $PACKAGE_DIR
    cd $PACKAGE_DIR
    mv -v avidemux avidemux_$BITS
    TARGETDIR=${TOP}/${PACKAGE_DIR}/avidemux_${BITS}
    if [ ! -e ${TARGETDIR}/platforms ]; then
        mkdir ${TARGETDIR}/platforms;
    fi
    if [ ! -e ${TARGETDIR}/styles ]; then
        mkdir ${TARGETDIR}/styles;
    fi
    cd ${MINGW}/bin
    if [ "x${external_liba52}" = "x1" ]; then
        cp -v liba52-*.dll $TARGETDIR
    fi    
    if [ "x${external_libass}" = "x1" ]; then
        cp -v libass-*.dll $TARGETDIR
    fi
    if [ "x${external_libmad}" = "x1" ]; then
        cp -v libmad-*.dll $TARGETDIR
    fi
    cp -v \
    libaom.dll \
    libbz2.dll \
    libcrypto-*.dll \
    libeay32.dll \
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
    libharfbuzz-*.dll \
    libiconv-*.dll \
    libintl-*.dll \
    libmp3lame-*.dll \
    libogg-*.dll \
    libopus-*.dll \
    libpcre2-16-*.dll \
    libpcre-*.dll \
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
    ssleay32.dll \
    xvidcore.dll \
    zlib1.dll \
    $TARGETDIR;
    cd $QT_HOME
    cp -v \
    bin/Qt5Core.dll \
    bin/Qt5Gui.dll \
    bin/Qt5Network.dll \
    bin/Qt5Widgets.dll \
    bin/Qt5WinExtras.dll \
    $TARGETDIR;
    cp -v \
    plugins/platforms/qminimal.dll \
    plugins/platforms/qwindows.dll \
    ${TARGETDIR}/platforms/;
    cp -v \
    plugins/styles/qwindowsvistastyle.dll \
    ${TARGETDIR}/styles/;
    mkdir ${TARGETDIR}/etc
    cp -rvL ${MINGW}/etc/fonts ${TARGETDIR}/etc
    cd $TARGETDIR
    find . -name "*.dll.a" -exec rm -v '{}' \;
    rm -Rf include
    cd ../
    zip -r avidemux_r${BUILDDATE}_win${BITS}Qt5.zip avidemux_$BITS
    rm -Rf avidemux_$BITS
    cd $TOP
    echo "Avidemux Windows package generated as ${TOP}/${PACKAGE_DIR}/avidemux_r${BUILDDATE}_win${BITS}Qt5.zip"
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
        --with-internal-libass)
            export external_libass=0
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
    rm -Rf ${MINGWDEV}/*
fi
if [ -e "$INSTALL_DIR" -a "x$do_core" = "x1" -a "x$do_qt" = "x1" ]; then
    rm -Rf "$INSTALL_DIR";
fi
mkdir -p $INSTALL_DIR
echo "Top dir : $TOP"

# Build and install Avidemux components to the destination dir

if [ "x$do_core" = "x1" ]; then 
    echo "** CORE **"
    Process buildMingwCore-${ARCH} ../avidemux_core
fi

if [ "x$do_qt" = "x1" ]; then
    echo "** QT **"
    Process buildMingwQt5-${ARCH} ../avidemux/qt4 "-DQT_HOME:STRING=$QT_HOME -DENABLE_QT5=true"
fi

if [ "x$do_cli" = "x1" ]; then
    echo "** CLI **"
    Process buildMingwCli-${ARCH} ../avidemux/cli
fi

if [ "x$do_plugins" = "x1" ]; then
    echo "** Plugins **"
    Process buildMingwPluginsCommon-${ARCH} ../avidemux_plugins "-DPLUGIN_UI=COMMON $EXTRA_CMAKE_DEFS"
fi

if [ "x$do_plugins" = "x1" -a "x$do_qt" = "x1" ]; then
    echo "** Plugins Qt **"
    Process buildMingwPluginsQt5-${ARCH} ../avidemux_plugins "-DPLUGIN_UI=QT4 -DQT_HOME:STRING=$QT_HOME -DENABLE_QT5=true $EXTRA_CMAKE_DEFS"
fi

if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ]; then
    echo "** Plugins CLI **"
    Process buildMingwPluginsCli-${ARCH} ../avidemux_plugins "-DPLUGIN_UI=CLI $EXTRA_CMAKE_DEFS"
fi

if [ "x$do_plugins" = "x1" ]; then
    echo "** Plugins Settings **"
    Process buildMingwPluginsSettings-${ARCH} ../avidemux_plugins "-DPLUGIN_UI=SETTINGS $EXTRA_CMAKE_DEFS"
fi

# Create a ZIP archive with avidemux and all the required libs

if [ "x$do_release_pkg" = "x1" ]; then
    create_release_package
fi

echo "** All done **"
cd $TOP
echo "** ALL DONE **"
