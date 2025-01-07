#!/bin/bash
# Bootstrapper to semi-automatically build avidemux from source on OSX
# (c) Mean 2009

# My configuration, hardcoded
export MYQT=/usr/local/Cellar/qt/6.7.3
export PATH=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib:$PATH
export LIBRARY_PATH="$LIBRARY_PATH:/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib" # Needed for Rust compilation and linking

fail() {
  echo "** Failed at $1 **"
  exit 1
}
setupPaths() {
  # Specify the the directory where you want to install avidemux (a.k.a. the cmake_install_prefix)
  # like export BASE_INSTALL_DIR="<full_path_to_installation>". This can be /usr/local (homebrew)
  if [ "x$create_app_bundle" = "x1" ]; then
    export BASE_INSTALL_DIR="/"
    export BASE_APP="${BUILDTOP}/Avidemux${API_VERSION}.app"
    export PREFIX="${BASE_APP}/Contents/Resources"
    if [ ! -e "${BASE_APP}/Contents/Resources" ]; then
      mkdir -p "${BASE_APP}/Contents/Resources"
    fi
  else
    export BASE_INSTALL_DIR="${BUILDTOP}/out"
    export BASE_APP="$BASE_INSTALL_DIR"
    export PREFIX="$BASE_INSTALL_DIR"
  fi
}
Process() {
  BASE=$1
  SOURCEDIR=$2
  EXTRA="$3"
  DEBUG=""
  #BUILDER="Unix Makefiles"
  BUILDER="Ninja"
  echo "**************** $1 *******************"
  if [ "x$debug" = "x1" ]; then
    DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug"
    BASE="${BASE}_debug"
    BUILDER="CodeBlocks - Unix Makefiles"
  fi
  BUILDDIR="${PWD}/${BASE}"
  FAKEROOT=""
  if [ -n "$FAKEROOT_DIR" ]; then
    FAKEROOT="-DFAKEROOT=\"${FAKEROOT_DIR}\""
  fi
  echo "Building in \"${BUILDDIR}\" from \"${SOURCEDIR}\" with EXTRA=<$EXTRA>, DEBUG=<$DEBUG>"
  if [ "x$do_rebuild" != x1 ]; then
    rm -Rf "$BUILDDIR"
  fi
  if [ ! -e "$BUILDDIR" ]; then
    mkdir "$BUILDDIR" || fail mkdir
  fi
  pushd "$BUILDDIR" >/dev/null
  cmake \
    -DCMAKE_OSX_ARCHITECTURES="x86_64" \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DAVIDEMUX_SOURCE_DIR="$SOURCEDIR" \
    -DAVIDEMUX_VERSION="$ADM_VERSION" \
    $FAKEROOT \
    $EXTRA \
    $FLAVOR \
    $DEBUG \
    -G "$BUILDER" \
    "$SOURCEDIR" || fail cmakeZ
  ninja >/tmp/log${BASE} || fail make
  if [ -n "$FAKEROOT_DIR" ]; then
    echo "** installing to $FAKEROOT_DIR **"
  else
    echo "** installing to $PREFIX **"
  fi
  DESTDIR="$FAKEROOT_DIR" ninja install || fail install
  popd >/dev/null
}
printModule() {
  value=$1
  name=$2
  if [ "x$value" = "x1" ]; then
    echo -e "\t$name will be built"
  else
    echo -e "\t$name will be skipped"
  fi
}
config() {
  setupPaths
  echo "Build configuration :"
  echo "******************* :"
  echo -n "Build type : "
  if [ "x$debug" = "x1" ]; then
    echo "Debug build"
  else
    echo "Release build"
  fi
  if [ "x$adm_version" = "x" ]; then
    export ADM_VERSION="${MAJOR}.${MINOR}.${PATCH}"
  else
    export ADM_VERSION=$adm_version
  fi
  echo "Avidemux version : $ADM_VERSION"
  if [ "x$create_app_bundle" != "x1" ]; then
    echo "No macOS app bundle will be created"
  fi
  if [ "x$do_rebuild" != "x1" -a "x$BASE_APP" != "x" ]; then
    rm -Rf $BASE_APP/*
  fi
  printModule $do_core Core
  printModule $do_qt4 Qt
  printModule $do_cli Cli
  printModule $do_plugins Plugins
}
usage() {
  echo "Bootstrap avidemux ${API_VERSION}:"
  echo "***********************"
  echo "  --help                  : Print usage"
  echo "  --no-bundle             : Don't create macOS app bundle structure"
  echo "  --nopkg                 : Don't make macOS app bundle self-contained and package it as DMG"
  echo "  --debug                 : Switch debugging on"
  echo "  --rebuild               : Preserve existing build directories"
  echo "  --output=NAME           : Specify a custom basename for dmg"
  echo "  --version=STRING        : Specify a custom Avidemux version string"
  echo "  --with-core             : Build core (default)"
  echo "  --without-core          : Don't build core"
  echo "  --with-cli              : Build cli (default)"
  echo "  --without-cli           : Don't build cli"
  echo "  --with-qt               : Build qt (default)"
  echo "  --without-qt            : Don't build qt"
  echo "  --with-plugins          : Build plugins (default)"
  echo "  --without-plugins       : Don't build plugins"
  echo "  --with-internal-liba52  : Use bundled liba52 (a52dec) instead of the system one"
  echo "  --with-external-libmad  : Use system libmad instead of the bundled one"
  echo "  --with-internal-libmp4v2: Use bundled libmp4v2 instead of the system one"
}
option_value() {
  echo $(echo $* | cut -d '=' -f 2-)
}
validate() {
  opt="$1"
  str="$2"
  if [ "$opt" = "adm_version" ]; then
    reg="[^a-zA-Z0-9_.-]"
    msg="Only alphanumeric characters, hyphen, underscore and period are allowed for Avidemux version, aborting."
  elif [ "$opt" = "output" ]; then
    reg="[^a-zA-Z0-9\ _.-]"
    msg="Only alphanumeric characters, space, hyphen, underscore and period are allowed for .dmg basename, aborting."
  else
    >&2 echo "incorrect usage of validate(), aborting."
    exit 1
  fi
  if [[ "$str" =~ $reg ]]; then
    >&2 echo $msg
    exit 1
  fi
}

echo "** BootStrapping avidemux **"

BUILDTOP=$PWD
if [[ $BUILDTOP = *" "* ]]; then
  echo "The build directory path \"${BUILDTOP}\" contains one or more spaces."
  echo "This is unsupported by FFmpeg configure."
  fail bootstrap
fi

SRCTOP=$(cd $(dirname "$0") && pwd)

echo "Top build dir : \"${BUILDTOP}\""
echo "Top source dir : \"${SRCTOP}\""

pushd "${SRCTOP}" >/dev/null

export MAJOR=$(cat avidemux_core/cmake/avidemuxVersion.cmake | grep "VERSION_MAJOR " | sed 's/..$//g' | sed 's/^.*"//g')
export MINOR=$(cat avidemux_core/cmake/avidemuxVersion.cmake | grep "VERSION_MINOR " | sed 's/..$//g' | sed 's/^.*"//g')
export PATCH=$(cat avidemux_core/cmake/avidemuxVersion.cmake | grep "VERSION_P " | sed 's/..$//g' | sed 's/^.*"//g')
export API_VERSION="${MAJOR}.${MINOR}"

DAT=$(date +"%y%m%d-%Hh%Mm")
gt=$(git log --format=oneline -1 | head -c 11)
REV="${DAT}_$gt"

popd >/dev/null

FLAVOR="-DENABLE_QT6=True"
QT_EXT="Qt6"

do_core=1
do_cli=1
do_qt4=1
do_plugins=1
do_rebuild=0
debug=0
create_app_bundle=1
create_dmg=1
external_liba52=1
external_libmad=0
external_libmp4v2=1

export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
export MACOSX_DEPLOYMENT_TARGET=$(xcrun --sdk macosx --show-sdk-version)

test -f $HOME/myCC && export COMPILER="-DCMAKE_C_COMPILER=$HOME/myCC -DCMAKE_CXX_COMPILER=$HOME/myC++"

# Could probably do it with getopts...
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
    do_rebuild=1
    ;;
  --no-bundle)
    create_app_bundle=0
    ;;
  --nopkg)
    create_dmg=0
    ;;
  --output=*)
    dmg_base=$(option_value "$config_option")
    ;;
  --version=*)
    adm_version=$(option_value "$config_option")
    ;;
  --without-qt)
    do_qt4=0
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
    do_qt4=1
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
  --with-internal-liba52)
    external_liba52=0
    ;;
  --with-external-libmad)
    external_libmad=1
    ;;
  --with-internal-libmp4v2)
    external_libmp4v2=0
    ;;
  *)
    echo "unknown parameter $config_option"
    usage
    exit 1
    ;;
  esac
  shift
done

# Probe case-sensitivity of the file system where the build directory is located.
. "${SRCTOP}/checkCaseSensitivity.sh"
isCaseSensitive || { echo "Error: build directory file system is not case-sensitive." && exit 1; }

validate adm_version "$adm_version" || exit 1
validate output "$dmg_base" || exit 1
config
# If the path to a custom Qt installation is passed via MYQT variable,
# check for a conflicting one from Homebrew.
if [ -n "$MYQT" ] && [ -f "/usr/local/homebrew/bin/qmake" ]; then
  echo -e "\n****************************************************************"
  echo -e "It seems that you have a Qt installation linked into /usr/local/homebrew,"
  echo -e "but MYQT variable is set. Please unlink it first by executing"
  echo -e "in case of Homebrew and e.g. Qt6\n"
  echo -e "\tbrew unlink qt6\n"
  echo -e "then rerun this script."
  echo -e "****************************************************************\n"
  exit 1
fi
# Homebrew offers formulae both for Qt5 and Qt6. When the version we want
# doesn't match the one linked into /usr/local/homebrew, the build will fail.
qmake_location=$(which qmake)
if [ "$qmake_location" = "/usr/local/homebrew/bin/qmake" ]; then
  linked_qt_version_major=$(qmake -query QT_VERSION)
  linked_qt_version_major=${linked_qt_version_major:0:1}
  if [ "${QT_EXT}" != "Qt${linked_qt_version_major}" ]; then
    echo -e "\n*********************** Fatal Error ****************************"
    echo -e "Qt version of the installation linked into /usr/local/homebrew does not"
    echo -e "match the one you are building for. If you use Homebrew, please"
    echo -e "brew link the matching one and rerun this script."
    echo -e "****************************************************************\n"
    exit 1
  fi
fi
if [ -n "$MYQT" ] && [ -f "${MYQT}/bin/qmake" ]; then
  export QTDIR="$MYQT" # needed for translations
else
  if [ "$qmake_location" != "/usr/local/homebrew/bin/qmake" ]; then
    echo -e "\n****************************************************************"
    echo -e "When using Qt6 from Homebrew, please make sure it is linked into"
    echo -e "/usr/local/homebrew by executing\n"
    echo -e "\tbrew link qt6\n"
    echo -e "as qmake at least in Qt 6.2.0 reports a wrong path to plugins."
    echo -e "****************************************************************\n"
    exit 1
  fi
  export QTDIR="/usr/local/homebrew/opt/qt@6"
fi
export PATH="$PATH":"${QTDIR}/bin"
if $(which -s qmake) && [ -f "${QTDIR}/bin/qmake" ]; then
  echo "Using $QTDIR as Qt install path"
else
  echo "Error: No matching qmake executable found, aborting."
  exit 1
fi
#
POSTFIX=""
if [ "x$debug" = "x1" ]; then
  POSTFIX="_debug"
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
DO_BUNDLE=""
FAKEROOT_DIR=""
if [ "x$create_app_bundle" = "x1" ]; then
  DO_BUNDLE="-DCREATE_BUNDLE=true"
else
  DO_BUNDLE="-UCREATE_BUNDLE"
fi
if [ "x$do_core" = "x1" ]; then
  echo "** CORE **"
  Process buildCore "${SRCTOP}/avidemux_core" $DO_BUNDLE
fi
if [ "x$do_qt4" = "x1" ]; then
  echo "** QT **"
  Process build${QT_EXT} "${SRCTOP}/avidemux/qt4" $DO_BUNDLE
fi
if [ "x$do_cli" = "x1" ]; then
  echo "** CLI **"
  Process buildCli "${SRCTOP}/avidemux/cli"
fi
if [ "x$do_plugins" = "x1" ]; then
  echo "** Plugins **"
  Process buildPluginsCommon "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=COMMON $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" -a "x$do_qt4" = "x1" ]; then
  echo "** Plugins Qt **"
  Process buildPlugins${QT_EXT} "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=QT4 EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ]; then
  echo "** Plugins CLI **"
  Process buildPluginsCLI "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=CLI $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" ]; then
  echo "** Plugins Settings **"
  Process buildPluginsSettings "${SRCTOP}/avidemux_plugins" "-DPLUGIN_UI=SETTINGS $EXTRA_CMAKE_DEFS"
fi
#
cd "$BUILDTOP"
if [ "x$create_app_bundle" = "x1" ]; then
  mkdir "${PREFIX}/fonts"
  cp "${SRCTOP}/cmake/osx/fonts.conf" "${PREFIX}/fonts"
  # Copy icons
  echo "Copying icons"
  cp "${SRCTOP}"/cmake/osx/*.icns "$PREFIX"
  mkdir -p "${PREFIX}"/../MacOS
  if [ -d "${PREFIX}"/../PlugIns ]; then
    rm -Rf "${PREFIX}"/../PlugIns
  fi
  mkdir -p "${PREFIX}"/../PlugIns
  # Symlink lib directory
  if [ -e "${PREFIX}"/../lib ]; then
    rm "${PREFIX}"/../lib
  fi
  ln -s "${PREFIX}/lib" "${PREFIX}"/../
  # Symlink Qt plugins
  ln -s "${QTDIR}/share/qt/plugins/platforms" "${PREFIX}"/../PlugIns/
  ln -s "${QTDIR}/share/qt/plugins/styles" "${PREFIX}"/../PlugIns/
  # Create qt.conf
  echo "[Paths]" >"${PREFIX}"/../Resources/qt.conf
  echo "Plugins = PlugIns" >>"${PREFIX}"/../Resources/qt.conf
  if [ "x$create_dmg" = "x1" ]; then
    if [ -e installer ]; then
      chmod -R +w installer || fail "making the old installer directory writable"
      rm -Rf installer || fail "removing the old installer directory"
    fi
    mkdir installer || fail "creating new installer directory"
    cd installer
    cmake \
      -DAVIDEMUX_VERSION="$ADM_VERSION" \
      -DAVIDEMUX_MAJOR_MINOR="${MAJOR}.${MINOR}" \
      -DDMG_BASENAME="$dmg_base" \
      -DBUILD_REV="$REV" \
      $FLAVOR \
      "${SRCTOP}/avidemux/osxInstaller" || fail "cmake"
    echo "** Preparing packaging **"
    make package
  fi
fi
echo "** ALL DONE **"
