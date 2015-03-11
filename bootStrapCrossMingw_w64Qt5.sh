#!/bin/bash
# ** Put your config here **
export PATH=$PATH:/mingw/bin
export CROSS_PREFIX=x86_64-w64-mingw32
export SDLDIR=/mingw
export MINGW=/mingw
export QT_HOME=/mingw/Qt/current
#export CFLAGS="-fpermissive"
export O_PARAL="-j 2"
export TOOLCHAIN_LOCATION=/mingw

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
        make  $PARAL VERBOSE=1 || fail make
        make install || fail make_install
}

echo "**BootStrapping avidemux **"
export TOP=$PWD
echo "Top dir : $TOP"
echo "** CORE **"
cd $TOP
export PARAL=""
Process buildMingwCore cross_mingw64_core
export PARAL="$O_PARAL"
echo "** QT4 **"
cd $TOP
Process buildMingwQt4 cross_mingw64_qt5
export PARAL="$O_PARAL"
Process buildMingwCli cross_mingw_cli 
#echo "** GTK **"
#cd $TOP
#Process buildGtk ../avidemux/gtk
echo "** Plugins **"
cd $TOP
Process buildMingwPluginsCommon cross_mingw64_plugins -DPLUGIN_UI=COMMON
Process buildMingwPluginsQt4 cross_mingw64_qt5_plugins -DPLUGIN_UI=QT4
Process buildMingwPluginsCli cross_mingw64_plugins -DPLUGIN_UI=CLI
Process buildMingwPluginsSettings cross_mingw64_plugins -DPLUGIN_UI=SETTINGS
echo "** All done **"
cd $TOP
echo "** ALL DONE **"
