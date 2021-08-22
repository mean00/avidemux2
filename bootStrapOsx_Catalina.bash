#!/bin/bash
# Bootstrapper to semi-automatically build avidemux from source on OSX
# (c) Mean 2009

export MAJOR=`cat cmake/avidemuxVersion.cmake | grep "VERSION_MAJOR " | sed 's/..$//g' | sed 's/^.*"//g'`
export MINOR=`cat cmake/avidemuxVersion.cmake | grep "VERSION_MINOR " | sed 's/..$//g' | sed 's/^.*"//g'`
export PATCH=`cat cmake/avidemuxVersion.cmake | grep "VERSION_P " | sed 's/..$//g' | sed 's/^.*"//g'`
export API_VERSION="${MAJOR}.${MINOR}"

DAT=`date +"%y%m%d-%Hh%Mm"`
gt=`git log --format=oneline -1 | head -c 11`
REV="${DAT}_$gt"

FLAVOR="-DENABLE_QT5=True"
qt_ext="Qt5"
packages_ext=""
do_core=1
do_cli=1
do_gtk=0   # Note that gtk is no fully longer supported on OSX. You are on your own here
do_qt4=1
do_plugins=1
do_rebuild=0
debug=0
create_app_bundle=1
create_dmg=1
external_libass=1
external_liba52=1
external_libmad=0
external_libmp4v2=1

# /usr/include is no more on Catalina
export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)
#export MACOSX_DEPLOYMENT_TARGET=$(xcrun --sdk macosx --show-sdk-version)
if [ "x$MACOSX_DEPLOYMENT_TARGET" = "x" ] ; then
    export MACOSX_DEPLOYMENT_TARGET=10.15
fi

test -f $HOME/myCC  && export COMPILER="-DCMAKE_C_COMPILER=$HOME/myCC -DCMAKE_CXX_COMPILER=$HOME/myC++"

fail()
{
        echo "** Failed at $1**"
        exit 1
}
setupPaths()
{
# Specify the the directory where you want to install avidemux (a.k.a. the cmake_install_prefix)
# like export BASE_INSTALL_DIR="<full_path_to_installation>". This can be /usr/local or /opt/local (macports) or /sw (Fink)
        if [ "x$create_app_bundle" = "x1" ] ; then
            export BASE_INSTALL_DIR="/"
            export BASE_APP="$PWD/Avidemux${API_VERSION}.app"
            export PREFIX="${BASE_APP}/Contents/Resources"
            if [ ! -e $BASE_APP/Contents/Resources ] ; then
                mkdir -p $BASE_APP/Contents/Resources
            fi
        else
            export BASE_INSTALL_DIR="$PWD/out"
            export BASE_APP="$BASE_INSTALL_DIR"
            export PREFIX="$BASE_INSTALL_DIR"
        fi
}
Process()
{
        BUILDDIR=$1
        SOURCEDIR=$2
        FAKEROOT="-DFAKEROOT=$FAKEROOT_DIR"
        EXTRA="$3"
        DEBUG=""
        BUILDER="Unix Makefiles"
        echo "**************** $1 *******************"
        if [ "x$debug" = "x1" ] ; then 
                DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug  "
                BUILDDIR="${BUILDDIR}_debug"
                BUILDER="CodeBlocks - Unix Makefiles"
        fi

        echo "Building $BUILDDIR from $SOURCEDIR with EXTRA=<$EXTRA>, DEBUG=<$DEBUG>"
        if [ "x$do_rebuild" != x1 ] ; then
            rm -Rf ./$BUILDDIR
        fi
        mkdir -p $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        cmake $COMPILER $PKG $FAKEROOT -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_EDIT_COMMAND=vim -DAVIDEMUX_SOURCE_DIR=$TOP -DAVIDEMUX_VERSION="$ADM_VERSION" $EXTRA $FLAVOR $DEBUG -G "$BUILDER" $SOURCEDIR || fail cmakeZ
        make -j 2 > /tmp/log$BUILDDIR || fail make
        echo "** installing at $FAKEROOT_DIR **"
        make install DESTDIR=$FAKEROOT_DIR || fail install
}
printModule()
{
        value=$1
        name=$2
        if [ "x$value" = "x1" ]; then echo "    $name will be built"
        else echo "     $name will be skipped"
        fi
}
config()
{
        setupPaths
        echo "Build configuration :"
        echo "******************* :"
        echo "Build type :"
        if [ "x$debug" = "x1" ] ; then echo   "Debug build"
        else echo   "Release build"
        fi
        if [ "x$adm_version" = "x" ] ; then
            export ADM_VERSION="${MAJOR}.${MINOR}.${PATCH}"
        else
            export ADM_VERSION=$adm_version
        fi
        echo "Avidemux version : $ADM_VERSION"
        if [ "x$create_app_bundle" != "x1" ] ; then
            echo "No macOS app bundle will be created"
        fi
        if [ "x$do_rebuild" != "x1" -a "x$BASE_APP" != "x" ] ; then
            rm -Rf $BASE_APP/*
        fi
        printModule $do_core Core
        printModule $do_gtk Gtk
        printModule $do_qt4 Qt
        printModule $do_cli Cli
        printModule $do_plugins Plugins
}
usage()
{
        echo "Bootstrap avidemux ${API_VERSION}:"
        echo "***********************"
        echo "  --help                  : Print usage"
        echo "  --no-bundle             : Don't create macOS app bundle structure"
        echo "  --nopkg                 : Don't make macOS app bundle self-contained and package it as DMG"
        echo "  --debug                 : Switch debugging on"
        echo "  --rebuild               : Preserve existing build directories"
        echo "  --output=NAME           : Specify a custom basename for dmg"
        echo "  --version=STRING        : Specify a custom Avidemux version string"
        echo "  --enable-qt6            : Require Qt6 instead of Qt5"
        echo "  --with-core             : Build core (default)"
        echo "  --without-core          : Don't build core"
        echo "  --with-cli              : Build cli (default)"
        echo "  --without-cli           : Don't build cli"
        echo "  --with-qt               : Build qt (default)"
        echo "  --without-qt            : Don't build qt"
        echo "  --with-plugins          : Build plugins (default)"
        echo "  --without-plugins       : Don't build plugins"
        echo "  --with-internal-libass  : Use bundled libass instead of the system one"
        echo "  --with-internal-liba52  : Use bundled liba52 (a52dec) instead of the system one"
        echo "  --with-external-libmad  : Use system libmad instead of the bundled one"
        echo "  --with-internal-libmp4v2: Use bundled libmp4v2 instead of the system one"
}
option_value()
{
        echo $(echo $* | cut -d '=' -f 2-)
}
validate()
{
        opt="$1"
        str="$2"
        if [ "$opt" = "adm_version" ] ; then
            reg="[^a-zA-Z0-9_.-]"
            msg="Only alphanumeric characters, hyphen, underscore and period are allowed for Avidemux version, aborting."
        elif [ "$opt" = "output" ] ; then
            reg="[^a-zA-Z0-9\ _.-]"
            msg="Only alphanumeric characters, space, hyphen, underscore and period are allowed for .dmg basename, aborting."
        else
            >&2 echo "incorrect usage of validate(), aborting."
            exit 1
        fi
        if [[ "$str" =~ $reg ]] ; then
            >&2 echo $msg
            exit 1
        fi
}
# Could probably do it with getopts...
while [ $# != 0 ] ;do
        config_option="$1"
        case "$config_option" in
         -h|--help)
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
                output=$(option_value "$config_option")
                ;;
         --version=*)
                adm_version=$(option_value "$config_option")
                ;;
         --enable-qt6)
                FLAVOR="-DENABLE_QT6=True"
                qt_ext=Qt6
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
         --with-internal-libass)
                external_libass=0
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
validate adm_version "$adm_version" || exit 1
validate output "$output" || exit 1
config
# Homebrew offers formulae both for Qt5 and Qt6. When the version we want
# doesn't match the one linked into /usr/local, the build will fail.
# Refuse to continue if Qt is not keg-only.
if [ -f "/usr/local/bin/qmake" ]; then
    echo -e "\n***************************************************************"
    echo -e "It seems that you have a Qt installation linked into /usr/local"
    echo -e "Please unlink it first, e.g. for Qt6 by executing\n"
    echo -e "\tbrew unlink qt6\n"
    echo -e "then rerun this script."
    echo -e "***************************************************************\n"
    exit 1
fi
if [ "x$MYQT" != "x" ] && [ -f "${MYQT}/bin/qmake" ] ; then
    export QTDIR="${MYQT}" # needed for translations
else
    if [ $qt_ext = "Qt6" ]; then
        export QTDIR=/usr/local/opt/qt6
    else
        export QTDIR=/usr/local/opt/qt5
    fi
fi
export PATH=${PATH}:${QTDIR}/bin
if $(which -s qmake) && [ -f "${QTDIR}/bin/qmake" ]; then
    echo "Using ${QTDIR} as Qt install path"
else
    echo "Error: No matching qmake executable found, aborting."
    exit 1
fi
#
echo "** BootStrapping avidemux **"
export TOP=$PWD
export POSTFIX=""
echo "Top dir : $TOP"
if [ "x$debug" = "x1" ] ; then echo
POSTFIX="_debug"
fi
if [ "x$external_libass" = "x1" ]; then
    export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBASS=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$external_liba52" = "x1" ]; then
    export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBA52=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$external_libmad" = "x1" ]; then
    export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_LIBMAD=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$external_libmp4v2" = "x1" ]; then
    export EXTRA_CMAKE_DEFS="-DUSE_EXTERNAL_MP4V2=true $EXTRA_CMAKE_DEFS"
fi
if [ "x$create_app_bundle" = "x1" ] ; then
    export DO_BUNDLE="-DCREATE_BUNDLE=true"
else
    export DO_BUNDLE="-UCREATE_BUNDLE"
fi
if [ "x$do_core" = "x1" ] ; then
        echo "** CORE **"
        cd $TOP
        Process buildCore ../avidemux_core $DO_BUNDLE
fi
if [ "x$do_qt4" = "x1" ] ; then
        echo "** QT **"
        cd $TOP
        Process build${qt_ext} ../avidemux/qt4 $DO_BUNDLE
fi
if [ "x$do_cli" = "x1" ] ; then
        echo "** CLI **"
        cd $TOP
        Process buildCli ../avidemux/cli
fi
if [ "x$do_plugins" = "x1" ] ; then
        echo "** Plugins **"
        cd $TOP
        Process buildPluginsCommon ../avidemux_plugins "-DPLUGIN_UI=COMMON $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" -a "x$do_qt4" = "x1" ] ; then
        echo "** Plugins Qt **"
        cd $TOP
        Process buildPlugins${qt_ext} ../avidemux_plugins "-DPLUGIN_UI=QT4 EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ] ; then
        echo "** Plugins CLI **"
        cd $TOP
        Process buildPluginsCLI ../avidemux_plugins "-DPLUGIN_UI=CLI $EXTRA_CMAKE_DEFS"
fi
if [ "x$do_plugins" = "x1" ] ; then
        echo "** Plugins Settings **"
        cd $TOP
        Process buildPluginsSettings ../avidemux_plugins "-DPLUGIN_UI=SETTINGS $EXTRA_CMAKE_DEFS"
fi
# 
cd $TOP
if [ "x$create_app_bundle" = "x1" ] ; then
    mkdir $PREFIX/fonts
    cp $TOP/cmake/osx/fonts.conf $PREFIX/fonts
    # Copy icons
    echo "Copying icons"
    cp $TOP/cmake/osx/*.icns $PREFIX/
    mkdir -p $PREFIX/../MacOS
    if [ -d $PREFIX/../PlugIns ]; then
        rm -Rf $PREFIX/../PlugIns
    fi
    mkdir -p $PREFIX/../PlugIns
    # Symlink lib directory
    if [ -e $PREFIX/../lib ]; then
        rm $PREFIX/../lib
    fi
    ln -s $PREFIX/lib $PREFIX/../
    # Symlink Qt plugins
    ln -s ${QTDIR}/share/qt/plugins/platforms $PREFIX/../PlugIns/
    ln -s ${QTDIR}/share/qt/plugins/styles $PREFIX/../PlugIns/
    # Create qt.conf
    echo "[Paths]" > $PREFIX/../Resources/qt.conf
    echo "Plugins = PlugIns" >> $PREFIX/../Resources/qt.conf
    if [ "x$create_dmg" = "x1" ] ; then
        if [ -e installer ]; then
            chmod -R +w installer || fail "making the old installer directory writable"
            rm -Rf installer || fail "removing the old installer directory"
        fi
        mkdir installer || fail "creating new installer directory"
        cd installer
        cmake -DAVIDEMUX_VERSION="$ADM_VERSION" -DDMG_BASENAME="$output" -DBUILD_REV="$REV" $FLAVOR ../avidemux/osxInstaller || fail "cmake"
        echo "** Preparing packaging **"
        make install && make package
    fi
fi
echo "** ALL DONE **"
