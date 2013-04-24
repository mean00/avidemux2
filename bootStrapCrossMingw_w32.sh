#!/bin/bash
# ** Put your config here **
export PATH=$PATH:/mingw/bin
export CROSS_PREFIX=i686-w64-mingw32
export SDLDIR=/mingw
export MINGW=/mingw
export TOOLCHAIN_LOCATION=/home/fx/mxe-octave/usr/i686-pc-mingw32/
export QT_HOME=/mingw/Qt/current
export O_PARAL="-j 2"
// 0x501 = windows XP
export CFLAGS="-DWINVER=0x501"
export CXXFLAGS="$CFLAGS"
# ** Put your config here **

fail()
{
        echo "** Failed at $1**"
        exit 1
}

Process()
{
        export BUILDDIR=$1
        export SCRIPT=$2
        export EXTRA=$3
        echo "Building $BUILDDIR from $SOURCEDIR (Extra=$EXTRA)"
        rm -Rf ./$BUILDDIR
        mkdir $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        sh $TOP/foreignBuilds/$SCRIPT $EXTRA || fail cmake
        make $PARAL VERBOSE=1 || fail make
        make install || fail make_install
}

echo "**BootStrapping avidemux **"
export TOP=$PWD
echo "Top dir : $TOP"
echo "** CORE **"
cd $TOP
Process buildMingwCore cross_mingw_core
echo "** QT4 **"
cd $TOP
export PARAL=""
Process buildMingwQt4 cross_mingw_qt4 
export PARAL="$O_PARAL"
Process buildMingwCli cross_mingw_cli 
#echo "** GTK **"
#cd $TOP
#Process buildGtk ../avidemux/gtk
echo "** Plugins **"
cd $TOP
Process buildMingwPluginsCommon cross_mingw_plugins -DPLUGIN_UI=COMMON
Process buildMingwPluginsQt4 cross_mingw_plugins -DPLUGIN_UI=QT4
Process buildMingwPluginsCli cross_mingw_plugins -DPLUGIN_UI=CLI
Process buildMingwPluginsSettings cross_mingw_plugins -DPLUGIN_UI=SETTINGS
echo "** All done **"
cd $TOP
echo "** ALL DONE **"
