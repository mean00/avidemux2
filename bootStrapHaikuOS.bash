#!/bin/bash
# Bootstrapper to semi-automatically build avidemux from source on OSX
# (c) Mean 2009

# Specify the the directory where you want to install avidemux (a.k.a. the cmake_install_prefix)
# like export BASE_INSTALL_DIR="<full_path_to_installation>". This can be /usr/local or /opt/local (macports) or /sw (Fink)
export BASE_INSTALL_DIR="/boot/apps/Avidemux";

packages_ext=""
do_core=1
do_cli=0
do_gtk=0   # Note that gtk is no fully longer supported on OSX. You are on your own here
do_qt4=1
do_plugins=1
debug=0
fail()
{
        echo "** Failed at $1**"
        exit 1
}


Process()
{
        export BUILDDIR=$1
        export SOURCEDIR=$2
        export EXTRA=$3
        export DEBUG=""
        BUILDER="Unix Makefiles"
        if [ "x$debug" = "x1" ] ; then 
                DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug  "
                BUILDDIR="${BUILDDIR}_debug"
                BUILDER="CodeBlocks - Unix Makefiles"
        fi
        
        echo "Building $BUILDDIR from $SOURCEDIR with EXTRA=<$EXTRA>, DEBUG=<$DEBUG>"
        rm -Rf ./$BUILDDIR
        mkdir $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        cmake $PKG -DCMAKE_CXX_FLAGS="-D__STDC_CONSTANT_MACROS" -DPTHREAD_INCLUDE_DIR=/boot/develop/headers/posix -DAVIDEMUX_SOURCE_DIR=$TOP -DCMAKE_INSTALL_PREFIX="$BASE_INSTALL_DIR" $EXTRA $DEBUG -G "$BUILDER" $SOURCEDIR || fail cmakeZ
        make -j 2 > /tmp/log$BUILDDIR || fail make
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
        printModule $do_qt4 Qt4
        printModule $do_cli Cli
        printModule $do_plugins Plugins
}
usage()
{
        echo "Bootstrap avidemux 2.6:"
        echo "***********************"
        echo "  --help            : Print usage"
        echo "  --tgz             : Build tgz packages"
        echo "  --debug           : Switch debugging on"
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
        config 

}
# Could probably do it with getopts...
while [ $# != 0 ] ;do
        case "$1" in
         -h|--help)
             usage
             exit 1
             ;;
         --debug)
                debug=1
                ;;
         --pause)
                pause_script=true
                ;;
         --tgz)
                packages_ext=tar.gz
                PKG="$PKG -DAVIDEMUX_PACKAGER=tgz"
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
echo "Top dir : $TOP"
if [ "x$debug" = "x1" ] ; then echo   
POSTFIX="_debug"
fi

if [ "x$do_core" = "x1" ] ; then 
        echo "** CORE **"
        cd $TOP
        Process buildCore ../avidemux_core
        echo " Core needs to be installed, installing through  make install ...."
        cd $TOP/buildCore${POSTFIX} &&  make install
fi
if [ "x$do_qt4" = "x1" ] ; then 
        echo "** QT4 **"
        cd $TOP
        Process buildQt4 ../avidemux/qt4
        echo " Qt4 needs to be installed, installing through  make install ...."
        cd $TOP/buildQt4${POSTFIX} &&  make install
fi
if [ "x$do_cli" = "x1" ] ; then 
        echo "** CLI **"
        cd $TOP
        Process buildCli ../avidemux/cli
        echo " Cli needs to be installed, installing through  make install ...."
        cd $TOP/buildCli${POSTFIX} &&  make install
fi
if [ "x$do_gtk" = "x1" ] ; then 
        echo "** GTK **"
        cd $TOP
        Process buildGtk ../avidemux/gtk
        echo " Gtk needs to be installed, installing through  make install ...."
        cd $TOP/buildGtk${POSTFIX} &&  make install
fi
if [ "x$do_plugins" = "x1" ] ; then 
        echo "** Plugins **"
        cd $TOP
        Process buildPluginsCommon ../avidemux_plugins -DPLUGIN_UI=COMMON
fi
if [ "x$do_plugins" = "x1" -a "x$do_qt4" = "x1" ] ; then 
        echo "** Plugins Qt4 **"
        cd $TOP
        Process buildPluginsQt4 ../avidemux_plugins -DPLUGIN_UI=QT4
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
