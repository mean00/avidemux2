#!/bin/bash
# Bootstrapper to semi-automatically build avidemux deb/rpm from source
# (c) Mean 2009/2016
#
# By default we use qt5 now
#
packages_ext=""
rebuild=0
do_core=1
do_cli=1
do_gtk=0  # Gtk is obsolete!
do_qt4=1
do_plugins=1
do_asan=0
debug=0
install_prefix="/usr"
qt_ext=Qt5
QT_FLAVOR="-DENABLE_QT5=True"
COMPILER=""
export QT_SELECT=5 # default for ubuntu, harmless for others
export O_PARAL="-j $(nproc)"
fail()
{
        echo "** Failed at $1**"
        exit 1
}

Process()
{
        export BUILDDIR=$1
        export SOURCEDIR=$2
        export INSTALL_PREFIX="-DCMAKE_INSTALL_PREFIX=$install_prefix"
        export EXTRA=$3
        export DEBUG=""
        export ASAN=""
        BUILDER="Unix Makefiles"
        if [ "x$debug" = "x1" ] ; then 
                DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug  "
                BUILDDIR="${BUILDDIR}_debug"
                BUILDER="CodeBlocks - Unix Makefiles"
        fi
        if [ "x$do_asan" = "x1" ] ; then 
                BUILDDIR="${BUILDDIR}_asan"
                ASAN="-DASAN=True"
        fi
	FAKEROOT=" -DFAKEROOT=$FAKEROOT_DIR "
        echo "Building $BUILDDIR from $SOURCEDIR with EXTRA=<$EXTRA>, DEBUG=<$DEBUG>"
        if [ "x$rebuild" != "x1" ] ; then
                rm -Rf ./$BUILDDIR
        fi
        if [ ! -e "$BUILDDIR" ] ; then
                mkdir $BUILDDIR || fail mkdir
        fi
        cd $BUILDDIR 
        cmake $COMPILER $PKG $FAKEROOT $QT_FLAVOR -DCMAKE_EDIT_COMMAND=vim $INSTALL_PREFIX $EXTRA $ASAN $DEBUG -G "$BUILDER" $SOURCEDIR || fail cmakeZ
        make  $PARAL >& /tmp/log$BUILDDIR || fail "make, result in /tmp/log$BUILDDIR"
	if  [ "x$PKG" != "x" ] ; then
          $FAKEROOT_COMMAND make package DESTDIR=$FAKEROOT_DIR/tmp || fail package
	fi
        # we need the make install so that other packcges can be built against this one
        make install DESTDIR=$FAKEROOT_DIR
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
        echo "Build configuration :"
        echo "******************* :"
        echo "Build type :"
        if [ "x$debug" = "x1" ] ; then echo   "Debug build"
        else echo   "Release build"
        fi
        printModule $do_core Core
        printModule $do_gtk Gtk
        printModule $do_qt4 ${qt_ext}
        printModule $do_cli Cli
        printModule $do_plugins Plugins
}
usage()
{
        echo "Bootstrap avidemux 2.6:"
        echo "***********************"
        echo "  --help            : Print usage"
        echo "  --prefix=DIR      : Install to directory DIR (default: /usr)"
        echo "  --rpm             : Build rpm packages"
        echo "  --deb             : Build deb packages"
        echo "  --tgz             : Build tgz packages"
        echo "  --debug           : Switch debugging on"
        echo "  --rebuild         : Preserve existing build directories"
        echo "  --with-core       : Build core"
        echo "  --without-core    : Dont build core"
        echo "  --with-cli        : Build cli"
        echo "  --without-cli     : Dont build cli"
        echo "  --with-gtk        : Build gtk"
        echo "  --without-gtk     : Dont build gtk"
        echo "  --with-core       : Build core"
        echo "  --without-qt4     : Dont build qt4"
        echo "  --with-plugins    : Build plugins"
        echo "  --without-plugins : Dont build plugins"
        echo "  --enable-qt4      : Try to use qt4 instead of qt5"
        echo "  --enable-asan     : Enable Clang/llvm address sanitizer"
        echo "  --with-clang      : Use clang/clang++ as compiler"
	echo "The end result will be in the install folder. You can then copy it to / or whatever"
        config 

}
option_value()
{
        echo $(echo $* | cut -d '=' -f 2-)
}
dir_check()
{
        if [ "x$1" != "x" ] ; then
            if [[ "$1" != /* ]] ; then
                >&2 echo "Expected an absolute path for --${option_name}=$1, aborting."
                exit 1
            fi
        else
            >&2 echo "Empty path provided for --${option_name}, aborting."
            exit 1
        fi
        case "$1" in
          */)
              echo $(expr "x$1" : 'x\(.*[^/]\)') # strip trailing slashes
              ;;
          *)
              echo "$1"
              ;;
        esac
}
#

export FAKEROOT_COMMAND="fakeroot"
CMAKE_VERSION=`cmake --version | head -n 1 | sed "s/^.* \([0-9]*\.[0-9]*\.[0-9]*\).*/\1/g"`
echo "CMAKE Version : $CMAKE_VERSION"
case "$CMAKE_VERSION" in
         2.8.[7-9]|2.8.[1-9][0-9]|3*)
                echo "Cmake version >=2.8.7 doesnt need fakeroot"
                export FAKEROOT_COMMAND=""
        ;;
esac
# Could probably do it with getopts...
while [ $# != 0 ] ;do
        case "$1" in
         -h|--help)
             usage
             exit 1
             ;;
         --prefix=*)
                option_name=prefix
                install_prefix=$(dir_check $(option_value "$1")) || exit 1
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
                rm -f debs/*
                ;;
         --rpm)
                packages_ext=rpm
                PKG="$PKG -DAVIDEMUX_PACKAGER=rpm"
                ;;
         --with-clang)
                export COMPILER="-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++"
                ;;
         --without-qt4)
                do_qt4=0
             ;;
         --without-cli)
                do_cli=0
             ;;
         --without-gtk)
                do_gtk=0
             ;;
         --without-plugins)
                do_plugins=0
             ;;
         --without-core)
                do_core=0
             ;;
         --enable-qt4)
                QT_FLAVOR=""
                export QT_SELECT=4
                qt_ext=Qt4
             ;;
         --enable-asan)
                do_asan=1
             ;;
         --with-qt4)
                do_qt4=1
             ;;
         --with-cli)
                do_cli=1
             ;;
         --with-gtk)
                do_gtk=1
             ;;
         --with-plugins)
                do_plugins=1
             ;;
         --with-core)
                do_core=1
             ;;
        *)
                echo "unknown parameter $1"
                usage
                exit 1
                ;;
     esac
     shift
done
config
echo "**BootStrapping avidemux **"

export TOP=$PWD
export POSTFIX=""
export FAKEROOT_DIR=$PWD/install
export PARAL="$O_PARAL"
echo "Top dir : $TOP"
echo "Fake installation directory=$FAKEROOT_DIR"
if [ "x$debug" = "x1" ] ; then echo   
POSTFIX="_debug"
fi
if [ "x$packages_ext" = "x" ]; then 
	echo ""	
else
	rm -Rf $FAKEROOT_DIR
	mkdir -p $FAKEROOT_DIR
        echo "Cleaning packages"
        find . -name "*.$packages_ext" | grep -vi cpa | xargs rm -f
fi

if [ "x$do_core" = "x1" ] ; then 
        echo "** CORE **"
        cd $TOP
        Process buildCore ../avidemux_core
        echo " Installing core"
        cd $TOP/buildCore${POSTFIX} 
fi
if [ "x$do_qt4" = "x1" ] ; then 
        echo "** $qt_ext **"
        cd $TOP
        Process build${qt_ext} ../avidemux/qt4
        echo " Installing ${qt_ext}"
        cd $TOP/build${qt_ext}${POSTFIX} 
fi
if [ "x$do_cli" = "x1" ] ; then 
        echo "** CLI **"
        cd $TOP
        Process buildCli ../avidemux/cli 
        echo " Installing cli"
        cd $TOP/buildCli${POSTFIX}  
fi
if [ "x$do_gtk" = "x1" ] ; then 
        echo "** GTK **"
        cd $TOP
        Process buildGtk ../avidemux/gtk 
        echo " Installing Gtk"
        cd $TOP/buildGtk${POSTFIX} 
fi
if [ "x$do_plugins" = "x1" ] ; then 
        echo "** Plugins **"
        cd $TOP
        Process buildPluginsCommon ../avidemux_plugins -DPLUGIN_UI=COMMON
fi
if [ "x$do_plugins" = "x1" -a "x$do_qt4" = "x1" ] ; then 
        echo "** Plugins ${qt_ext} **"
        cd $TOP
        Process buildPlugins${qt_ext} ../avidemux_plugins -DPLUGIN_UI=QT4
fi
if [ "x$do_plugins" = "x1" -a "x$do_gtk" = "x1" ] ; then 
        echo "** Plugins Gtk **"
        cd $TOP
        Process buildPluginsGtk ../avidemux_plugins -DPLUGIN_UI=GTK
fi
if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ] ; then 
        echo "** Plugins CLI **"
        cd $TOP
        Process buildPluginsCLI ../avidemux_plugins -DPLUGIN_UI=CLI
fi
if [ "x$do_plugins" = "x1"  ] ; then 
        echo "** Plugins Settings **"
        cd $TOP
        Process buildPluginsSettings ../avidemux_plugins -DPLUGIN_UI=SETTINGS
fi


echo "** Preparing debs **"
cd $TOP
if [ "x$packages_ext" = "x" ]; then 
        echo "No packaging"
else
        echo "Preparing packages"
        rm -Rf debs
        mkdir debs
        find . -name "*.$packages_ext" | grep -vi cpa | xargs cp -t debs
        echo "** debs directory ready **"
        ls -l debs
fi
echo "** ALL DONE **"
if [ "x$packages_ext" = "x" ]; then 
    echo "** Copy the $FAKEROOT_DIR folder to your favorite location, i.e. sudo cp -R install/usr/* /usr/ **"
else
    echo "** The installable packages are in the debs folder **"
fi
