#!/bin/bash

# Default config

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

# Functions
export ARCH="x86_64"
export MXE_ROOT="$mxerootdir"
export MXE_TARGET=${ARCH}-w64-mingw32.shared
export QT_SELECT=6
export MINGW="${MXE_ROOT}/usr/${MXE_TARGET}"
export QT_HOME="${MINGW}"/qt6
export QTDIR=${mxerootdir}/usr/x86_64-pc-linux-gnu/qt6/
export TOOLCHAIN_LOCATION="${MXE_ROOT}"/usr
export TOOLCHAIN_FILE="${MINGW}"/share/cmake/mxe-conf.cmake
export SDL2DIR="$MINGW"
export GENERATOR="Ninja"

export PATH=${QT_HOME}/bin:$PATH

setupEnv() {
  export BITS="64"
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
  export PATH="$PATH":"${MXE_ROOT}"/usr/bin:"${QT_HOME}"/bin
  PARAL="-j $(nproc)"
  if [ "x$debug" != "x1" ]; then
    export INSTALL_DIR="${MINGW}"/out/avidemux
  else
    export INSTALL_DIR="${MINGW}"/out_debug/avidemux
  fi
  export CROSS_PREFIX=$MXE_TARGET
  export PKG_CONFIG_PATH="${MINGW}"/lib/pkgconfig
  export PKG_CONFIG_LIBDIR="${MINGW}"/lib/pkgconfig
  export CXXFLAGS="-std=c++17"
  export CROSS_C_COMPILER=gcc
  export CROSS_CXX_COMPILER=g++
}

fail() {
  echo "** Failed at $1 **"
  exit 1
}

Process() {
  BASE=$1
  SOURCEDIR=$2
  EXTRA=$3
  BUILDDIR="${PWD}/${BASE}"
  echo "Building in \"${BUILDDIR}\" from \"${SOURCEDIR}\" with EXTRA=<${EXTRA}>)"
  if [ "x$rebuild" != "x1" ]; then
    rm -Rf "$BUILDDIR"
  fi
  if [ ! -e "$BUILDDIR" ]; then
    mkdir "$BUILDDIR" || fail "creating build directory"
  fi
  pushd "$BUILDDIR" >/dev/null
  cmake -DCROSS="$MINGW" \
    -DCMAKE_SYSTEM_NAME:STRING=Windows \
    -DCMAKE_FIND_ROOT_PATH="$MINGW" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_LINKER:STRING="${CROSS_PREFIX}-ld" \
    -DCMAKE_AR:STRING="${CROSS_PREFIX}-ar" \
    -DAVIDEMUX_TOP_SOURCE_DIR:STRING="$SRCTOP" \
    ${DEBUG} \
    -G "${GENERATOR}" \
    $EXTRA \
    "$SOURCEDIR" || fail "cmake"
  ninja >&/tmp/log$BASE || fail "make, result in /tmp/log$BASE"
  ninja install || fail "install"
  # Only install  component=dev for dev package
  popd >/dev/null
}

usage() {
  echo "Usage: bash $0 [OPTION]"
  echo "  --help                 : Print usage"
  echo "  --debug                : Switch debugging on"
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
}

option_value() {
  echo $(echo $* | cut -d '=' -f 2-)
}

option_name() {
  echo $(echo $* | cut -d '=' -f 1 | cut -b 3-)
}

# Options handling

while [ $# != 0 ]; do
  config_option="$1"
  case "$config_option" in
  -h | --help)
    usage
    exit 1
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
if [ -e "$INSTALL_DIR" -a "x$do_core" = "x1" -a "x$do_qt" = "x1" ]; then
  rm -Rf "$INSTALL_DIR"
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
  Process buildMingwQt6-${ARCH} "${SRCTOP}"/avidemux/qt4 "-DQT_HOME:STRING=${QT_HOME} -DENABLE_QT6=true"
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
  Process buildMingwPluginsQt6-${ARCH} "${SRCTOP}"/avidemux_plugins "-DPLUGIN_UI=QT4 -DQT_HOME:STRING=$QT_HOME -DENABLE_QT6=true $EXTRA_CMAKE_DEFS"
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

rm -Rf avidemux64
cp -Rap ${MINGW}/out/avidemux ./avidemux64
find avidemux64 -name "*.a" | xargs rm -f
rm -Rf avidemux64/include
mv avidemux64/avidemux.exe avidemux64/avidemux_portable.exe
#  Copy extra QT6 files
mkdir avidemux64/platforms
cp ${QT_HOME}/plugins/platforms/qminimal.dll ${QT_HOME}/plugins/platforms/qwindows.dll avidemux64/platforms

mkdir avidemux64/styles
cp ${QT_HOME}/plugins/styles/qmodernwindowsstyle.dll avidemux64/styles

mkdir avidemux64/etc
cp -rvL "${MINGW}"/etc/fonts avidemux64/etc

#
python3 mxe_scan_deps.py \
  avidemux64 \
  --sources \
  ${MINGW}/bin \
  ${MINGW}/bin \
  ${QT_HOME}/bin \
  ${QT_HOME}/lib

#if [ "x$do_release_pkg" = "x1" ]; then
#create_release_package
#fi

echo "** All done **"
