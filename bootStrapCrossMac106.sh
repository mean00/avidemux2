#!/bin/bash
# ** Put your config here **
export CROSS_PREFIX=i686-apple-darwin10
export SDLDIR=/opt/mac
export TOOLCHAIN_DIR=/opt/mac/bin
export XSDK=/opt/mac/SDKs/MacOSX10.6.sdk/usr
export QT_HOME=/opt/mac/Qt/4.5.2
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
        echo "Building $BUILDDIR from $SOURCEDIR"
        rm -Rf ./$BUILDDIR
        mkdir $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        sh $TOP/foreignBuilds/$SCRIPT || fail cmake
        make -j 2 > /tmp/log$BUILDDIR || fail make
        make install || fail make_install
}

echo "**BootStrapping avidemux **"
export TOP=$PWD
echo "Top dir : $TOP"
echo "** CORE **"
cd $TOP
Process buildMingwCore cross_mac104_core
echo "** QT4 **"
cd $TOP
Process buildMingwQt4 cross_mac104_qt4 
#echo "** GTK **"
#cd $TOP
#Process buildGtk ../avidemux/gtk
echo "** Plugins **"
cd $TOP
Process buildMingwPlugins cross_mac104_plugins 
echo "** All done **"
cd $TOP
echo "** ALL DONE **"
