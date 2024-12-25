#!/bin/bash
# Bootstrapper to semi-automatically build avidemux deb/rpm from source
# (c) Mean-Euma  2009/2024
#
# By default we use qt5 now
#
#
RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"
#
packages_ext=""
packages_dir="pkgs"
rebuild=0
do_ninja=0
do_core=1
do_cli=1
do_qt=1
do_plugins=1
do_asan=0
debug=0
default_install_prefix="/usr"
qt_ext=Qt5
QT_FLAVOR="-DENABLE_QT5=True"
COMPILER=""
export QT_SELECT=5 # default for ubuntu, harmless for others
install_prefix="$default_install_prefix"
# -lc is required to build libADM_ae_lav* audio encoder plugins on 32 bit ubuntu
need_ae_lav_build_quirk=""
if [[ $(uname -m) = i?86 ]]; then
  need_ae_lav_build_quirk="1"
fi
external_libass=0
external_liba52=0
external_libmad=0
external_libmp4v2=0
#test -f $HOME/myCC && export COMPILER="-DCMAKE_C_COMPILER=$HOME/myCC -DCMAKE_CXX_COMPILER=$HOME/myC++"

fail() {
  echo -e "${RED}** Failed at $1 **${ENDCOLOR}"
  exit 1
}

Process() {
  BASE=$1
  SOURCEDIR=$2
  INSTALL_PREFIX="-DCMAKE_INSTALL_PREFIX=$install_prefix"
  EXTRA=$3
  DEBUG=""
  ASAN=""
  BUILD_QUIRKS=""
  if [ "x$do_ninja" = "x1" ]; then
    BUILDER="Ninja"
    MAKER="ninja"
  else
    BUILDER="Unix Makefiles"
    MAKER="make -j $(nproc)"
  fi
  if [ "x$debug" = "x1" ]; then
    DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug"
    BASE="${BASE}_debug"
  fi
  if [ "x$do_asan" = "x1" ]; then
    BASE="${BASE}_asan"
    ASAN="-DASAN=True"
  fi
  BUILDDIR="${PWD}/${BASE}"
  FAKEROOT="-DFAKEROOT=$FAKEROOT_DIR"
  echo -e "${GREEN}${BASE}: Building in \"${BUILDDIR}\" from \"${SOURCEDIR}\" with EXTRA=<$EXTRA>, DEBUG=<$DEBUG>, MAKER=<${MAKER}> ${ENDCOLOR}"
  echo "   $BASE:Cmake started..."
  $ADATE
  if [ "x$rebuild" != "x1" ]; then
    rm -Rf "${BUILDDIR}"
  fi
  if [ "x$need_ae_lav_build_quirk" = "x1" ]; then
    BUILD_QUIRKS="-DAE_LAVCODEC_BUILD_QUIRK=true"
  fi
  if [ ! -e "$BUILDDIR" ]; then
    mkdir "${BUILDDIR}" || fail "creating build directory"
  fi
  pushd "${BUILDDIR}" >/dev/null
  cmake \
    $COMPILER \
    $PKG \
    $FAKEROOT \
    $INSTALL_PREFIX \
    $EXTRA \
    $BUILD_QUIRKS \
    $ASAN \
    $DEBUG \
    -G "$BUILDER" \
    "$SOURCEDIR" >&/tmp/logCmake$BASE || fail "cmake,result in /tmp/logCmake$BASE"
  $ADATE
  echo "   $BASE:Build started..."
  ${MAKER} >&/tmp/log$BASE || fail "${MAKER}, result in /tmp/log$BASE"
  if [ "x$PKG" != "x" ]; then
    DESTDIR="${FAKEROOT_DIR}/tmp" $FAKEROOT_COMMAND ${MAKER} package || fail "packaging"
  fi
  $ADATE
  echo "   $BASE:Install started..."
  # we need the make install so that other packcges can be built against this one
  DESTDIR="${FAKEROOT_DIR}" ${MAKER} install >&/tmp/logInstall$BASE || fail "install faied, see /tmp/logInstall$BASE"
  popd >/dev/null
  $ADATE
  echo "   Done "
}
printModule() {
  value=$1
  name=$2
  if [ "x$value" = "x1" ]; then
    echo "    $name will be built"
  else
    echo "     $name will be skipped"
  fi

}
config() {
  echo "Build configuration :"
  echo "******************* :"
  echo "Build type :"
  if [ "x$debug" = "x1" ]; then
    echo "Debug build"
  else
    echo "Release build"
  fi
  printModule $do_core Core
  printModule $do_qt ${qt_ext}
  printModule $do_cli Cli
  printModule $do_plugins Plugins
}
usage() {
  echo "Bootstrap avidemux 2.6:"
  echo "***********************"
  echo "  --help                : Print usage"
  echo "  --prefix=DIR          : Install to directory DIR (default: $default_install_prefix)"
  echo "  --rpm                 : Create rpm packages"
  echo "  --deb                 : Create deb packages"
  echo "  --tgz                 : Create tgz packages"
  echo "  --debug               : Switch debugging on"
  echo "  --rebuild             : Preserve existing build directories"
  echo "  --with-ninja          : Build with ninja (default is make)"
  echo "  --with-core           : Build core (default)"
  echo "  --without-core        : Don't build core"
  echo "  --with-cli            : Build cli (default)"
  echo "  --without-cli         : Don't build cli"
  echo "  --with-qt             : Build Qt dependent components (default)"
  echo "  --without-qt          : Don't build Qt dependent components"
  echo "  --with-plugins        : Build plugins (default)"
  echo "  --without-plugins     : Don't build plugins"
  echo "  --enable-qt4          : Try to use Qt4 instead of Qt5"
  echo "  --enable-qt6          : Try to use Qt6 instead of Qt5"
  echo "  --enable-asan         : Enable Clang/llvm address sanitizer"
  echo "  --with-clang          : Use clang/clang++ as compiler"
  echo "  --with-system-libass  : Use system libass instead of the bundled one"
  echo "  --with-system-liba52  : Use system liba52 (a52dec) instead of the bundled one"
  echo "  --with-system-libmad  : Use system libmad instead of the bundled one"
  echo "  --with-system-libmp4v2: Use system libmp4v2 instead of the bundled one"
  echo "The end result will be in the install folder. You can then copy it to / or whatever"
  config

}
option_value() {
  echo $(echo $* | cut -d '=' -f 2-)
}
option_name() {
  echo $(echo $* | cut -d '=' -f 1 | cut -b 3-)
}
dir_check() {
  op_name="$1"
  dir_path="$2"
  if [ "x$dir_path" != "x" ]; then
    if [[ "$dir_path" != /* ]]; then
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
#

export FAKEROOT_COMMAND="fakeroot"
CMAKE_VERSION=$(cmake --version | head -n 1 | sed "s/^.* \([0-9]*\.[0-9]*\.[0-9]*\).*/\1/g")
echo "CMAKE Version : $CMAKE_VERSION"
case "$CMAKE_VERSION" in
2.8.[7-9] | 2.8.[1-9][0-9] | 3*)
  echo "Cmake version >=2.8.7 doesnt need fakeroot"
  export FAKEROOT_COMMAND=""
  ;;
esac
# Could probably do it with getopts...
while [ $# != 0 ]; do
  config_option="$1"
  case "$config_option" in
  -h | --help)
    usage
    exit 1
    ;;
  --prefix=*)
    install_prefix=$(dir_check $(option_name "$config_option") $(option_value "$config_option")) || exit 1
    ;;
  --debug)
    debug=1
    ;;
  --rebuild)
    rebuild=1
    ;;
  --tgz)
    packages_ext=tar.gz
    PKG="$PKG -DAVIDEMUX_PACKAGER=tgz"
    ;;
  --deb)
    packages_ext=deb
    PKG="$PKG -DAVIDEMUX_PACKAGER=deb"
    ;;
  --rpm)
    packages_ext=rpm
    PKG="$PKG -DAVIDEMUX_PACKAGER=rpm"
    ;;
  --with-clang)
    export COMPILER="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
    ;;
  --with-ninja)
    echo "Enabling ninja build"
    do_ninja=1
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
  --enable-qt4)
    QT_FLAVOR="-DENABLE_QT4=True"
    export QT_SELECT=4
    qt_ext=Qt4
    ;;
  --enable-qt6)
    QT_FLAVOR="-DENABLE_QT6=True"
    export QT_SELECT=6
    qt_ext=Qt6
    ;;
  --enable-asan)
    do_asan=1
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
  --with-system-libass)
    external_libass=1
    ;;
  --with-system-liba52)
    external_liba52=1
    ;;
  --with-system-libmad)
    external_libmad=1
    ;;
  --with-system-libmp4v2)
    external_libmp4v2=1
    ;;
  *)
    echo "unknown parameter $config_option"
    usage
    exit 1
    ;;
  esac
  shift
done
config
echo "**BootStrapping avidemux **"

BUILDTOP=$PWD
if [[ $BUILDTOP = *" "* ]]; then
  echo "The build directory path \"${BUILDTOP}\" contains one or more spaces."
  echo "This is unsupported by FFmpeg configure."
  fail "build prerequisites"
fi
SRCTOP=$(cd $(dirname "$0") && pwd)
POSTFIX=""
FAKEROOT_DIR="${BUILDTOP}/install"
if [ "x$external_libass" = "x1" ]; then
  EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBASS=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$external_liba52" = "x1" ]; then
  EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBA52=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$external_libmad" = "x1" ]; then
  EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBMAD=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$external_libmp4v2" = "x1" ]; then
  EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_MP4V2=true $EXTRA_CMAKE_DEFS"
fi
echo "Build top dir : \"${BUILDTOP}\""
echo "Fake installation directory : \"${FAKEROOT_DIR}\""
if [ "x$debug" = "x1" ]; then
  POSTFIX="_debug"
fi
if [ "x$packages_ext" = "x" ]; then
  echo ""
else
  rm -Rf "${FAKEROOT_DIR}"
  mkdir -p "${FAKEROOT_DIR}"
  echo "Cleaning packages"
  find . -name "*.$packages_ext" | grep -vi cpa | xargs rm -f
fi

if [ "x$do_core" = "x1" ]; then
  echo "** CORE **"
  Process buildCore "${SRCTOP}/avidemux_core"
fi
if [ "x$do_qt" = "x1" ]; then
  echo "** $qt_ext **"
  Process build${qt_ext} "${SRCTOP}/avidemux/qt4" "-DOpenGL_GL_PREFERENCE=GLVND $QT_FLAVOR"
fi
if [ "x$do_cli" = "x1" ]; then
  echo "** CLI **"
  Process buildCli "${SRCTOP}/avidemux/cli"
fi
if [ "x$do_plugins" = "x1" ]; then
  echo "** Plugins **"
  Process buildPluginsCommon "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=COMMON $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" -a "x$do_qt" = "x1" ]; then
  echo "** Plugins ${qt_ext} **"
  Process buildPlugins${qt_ext} "${SRCTOP}/avidemux_plugins" "-DOpenGL_GL_PREFERENCE=GLVND -DPLUGIN_UI=QT4 $QT_FLAVOR $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ]; then
  echo "** Plugins CLI **"
  Process buildPluginsCLI "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=CLI $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" ]; then
  echo "** Plugins Settings **"
  Process buildPluginsSettings "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=SETTINGS $EXTRA_CMAKE_DEFS"
fi

echo "** Packaging **"
cd "${BUILDTOP}"
if [ "x$packages_ext" = "x" ]; then
  echo "No packaging"
else
  echo -e "${GREEN}Preparing packages${ENDCOLOR}"
  rm -Rf "${packages_dir}"
  mkdir "${packages_dir}"
  find . -name "*.$packages_ext" | grep -vi cpa | xargs cp -t "${packages_dir}"
  echo "** ${packages_dir} directory ready **"
  ls -l "${packages_dir}"
fi
echo "** ALL DONE **"
if [ "x$packages_ext" = "x" ]; then
  echo "** Copy the folder \"${FAKEROOT_DIR}\" to your favorite location, i.e. sudo cp -R install/usr/* /usr/ **"
else
  echo "** Installable packages are in the ${packages_dir} folder **"
fi
