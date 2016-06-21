#!/bin/bash
# ** Put your config here **
export QT_SELECT=5
export PATH=$PATH:/mingw/bin
export CROSS_PREFIX=x86_64-w64-mingw32
export SDLDIR=/mingw
export MINGW=/mingw
export MINGWDEV=/mingw_dev
export QT_HOME=/mingw/Qt/current
export CFLAGS="-I/mingw/include -L/mingw/lib"
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
	make preinstall
        make install || fail make_install
        # Only install  component=dev for dev package
	DESTDIR=${MINGWDEV} cmake -DCOMPONENT=dev -P cmake_install.cmake || fail make_install_dev
}
# Clang ?

if [ "${USE_CLANG}" = "" ]; then
        export C_COMPILER=gcc
        export CXX_COMPILER=g++
else
        export C_COMPILER=clang
        export CXX_COMPILER=clang++
fi
# --
echo "**BootStrapping avidemux **"
rm -Rf ${MINGWDEV}
rm -Rf ${MINGW}/Release
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
