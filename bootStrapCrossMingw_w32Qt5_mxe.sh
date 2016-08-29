#!/bin/bash
# ** Put your config here **
export QT_SELECT=5
export PATH=$PATH:/mingw/bin
export CROSS_PREFIX=i686-w64-mingw32.shared
export SDLDIR=/mingw
export MINGW=/mingw
export MINGWDEV=/mingw_dev
export QT_HOME=/mingw/Qt/current
export CXXFLAGS="-std=c++11"
export O_PARAL="-j 2"
export TOOLCHAIN_LOCATION=/mingw
export CROSS_C_COMPILER=gcc
export CROSS_CXX_COMPILER=g++

# ** Put your config here **

fail()
{
        echo "** Failed at $1**"
        exit 1
}

Process()
{
	cd $TOP
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
        # Only install  component=dev for dev package
	DESTDIR=${MINGWDEV} cmake -DCOMPONENT=dev -P cmake_install.cmake || fail make_install_dev
}

echo "**BootStrapping avidemux **"
rm -Rf ${MINGWDEV}/*
rm -Rf ${MINGW}/Release
mkdir -p ${MINGW}/Release
export TOP=$PWD
export PARAL="$O_PARAL"
echo "Top dir : $TOP"
echo "** CORE **"
cd $TOP
Process buildMingwCore cross_mingw64_core
echo "** QT4 **"
cd $TOP
Process buildMingwQt4 cross_mingw64_qt5
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
