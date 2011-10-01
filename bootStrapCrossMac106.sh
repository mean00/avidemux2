#!/bin/bash
# ** Put your config here **
export CROSS_PREFIX=i686-apple-darwin10
export SDLDIR=/opt/mac
export TOOLCHAIN_DIR=/opt/mac/bin
export XSDK=/opt/mac/SDKs/MacOSX10.6.sdk/usr
export QT_HOME=/opt/mac/SDKs/MacOSX10.6.sdk/Library/Frameworks/
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
        make   || fail make
        #make -j 2 > /tmp/log$BUILDDIR || fail make
        make install || fail make_install
}

echo "**BootStrapping avidemux **"
export TOP=$PWD
echo "Top dir : $TOP"
echo "** CORE **"
cd $TOP
Process buildMacCore cross_mac104_core
echo "** QT4 **"
cd $TOP
Process buildMacQt4 cross_mac104_qt4 
#echo "** GTK **"
#cd $TOP
Process buildMacGtk ../avidemux/gtk
echo "** Plugins **"
cd $TOP
#Process buildMacPluginsCommon cross_mingw_plugins -DPLUGIN_UI=COMMON
#Process buildMacPluginsQt4 cross_mingw_plugins -DPLUGIN_UI=QT4
echo "** All done **"
cd $TOP
echo "** ALL DONE **"
