#!/bin/bash
# Bootstrapper to semi-automatically build avidemux from source on OSX
# (c) Mean 2009
fallback_qtdir=/usr/local/opt/qt5
if [ "x$MYQT" != "x" ] && [ -e "${MYQT}/bin/qmake" ] ; then
    export PATH=$PATH:$MYQT/bin:/opt/local/libexec/qt5/bin # for macports; /usr/local/bin is in PATH by default anyway
else
    export PATH=$PATH:/opt/local/libexec/qt5/bin
fi
if ! $(which -s qmake) && [ -e "${fallback_qtdir}/bin/qmake" ] ; then
    echo "Using ${fallback_qtdir} as fallback qt5 install path"
    export PATH=$PATH:${fallback_qtdir}/bin
fi
if ! $(which -s qmake) ; then
    echo "Error: No qmake executable found, aborting."
    exit 1
fi

export MAJOR=`cat cmake/avidemuxVersion.cmake | grep "VERSION_MAJOR " | sed 's/..$//g' | sed 's/^.*"//g'`
export MINOR=`cat cmake/avidemuxVersion.cmake | grep "VERSION_MINOR " | sed 's/..$//g' | sed 's/^.*"//g'`
export PATCH=`cat cmake/avidemuxVersion.cmake | grep "VERSION_P " | sed 's/..$//g' | sed 's/^.*"//g'`
export API_VERSION="${MAJOR}.${MINOR}"
export DAT=`date +"%y%m%d-%Hh%Mm"`
export gt=`git log --format=oneline -1 | head -c 11`
export REV="${DAT}_$gt"
#
# To move as parameter
#
export FLAVOR="-DENABLE_QT5=True"
export qt_ext=Qt5
#
packages_ext=""
do_core=1
do_cli=1
do_gtk=0   # Note that gtk is no fully longer supported on OSX. You are on your own here
do_qt4=1
do_plugins=1
do_rebuild=0
debug=0
create_app_bundle=1
external_libass=0
external_liba52=0
external_libmad=0
external_libmp4v2=0
#
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
        export BUILDDIR=$1
        export SOURCEDIR=$2
        export FAKEROOT="-DFAKEROOT=$FAKEROOT_DIR"
        export EXTRA="$3"
        export DEBUG=""
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
        echo "  --help                : Print usage"
        echo "  --tgz                 : Build tgz packages"
        echo "  --nopkg               : Don't create macOS app bundle"
        echo "  --debug               : Switch debugging on"
        echo "  --rebuild             : Preserve existing build directories"
        echo "  --output=NAME         : Specify a custom basename for dmg"
        echo "  --version=STRING      : Specify a custom Avidemux version string"
        echo "  --with-core           : Build core (default)"
        echo "  --without-core        : Don't build core"
        echo "  --with-cli            : Build cli (default)"
        echo "  --without-cli         : Don't build cli"
        echo "  --with-qt             : Build qt (default)"
        echo "  --without-qt          : Don't build qt"
        echo "  --with-plugins        : Build plugins (default)"
        echo "  --without-plugins     : Don't build plugins"
        echo "  --with-system-libass  : Use system libass instead of the bundled one"
        echo "  --with-system-liba52  : Use system liba52 (a52dec) instead of the bundled one"
        echo "  --with-system-libmad  : Use system libmad instead of the bundled one"
        echo "  --with-system-libmp4v2: Use system libmp4v2 instead of the bundled one"
        config
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
         --tgz)
                packages_ext=tar.gz
                PKG="$PKG -DAVIDEMUX_PACKAGER=tgz"
                ;;
         --nopkg)
                create_app_bundle=0
                ;;
         --output=*)
                output=$(option_value "$config_option")
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
validate adm_version "$adm_version" || exit 1
validate output "$output" || exit 1
config
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
    mkdir -p installer
    rm -Rf installer/*
    cd installer
    cmake -DAVIDEMUX_VERSION="$ADM_VERSION" -DDMG_BASENAME="$output" -DBUILD_REV="$REV" ../avidemux/osxInstaller
    make && make package
echo "** Preparing packaging **"
fi
echo "** ALL DONE **"
