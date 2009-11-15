#!/bin/bash
# ** Put your config here **
export CROSS_PREFIX=i586-mingw32msvc
export SDLDIR=/mingw
export MINGW=/mingw
export QT_HOME=/mingw/Qt/4.5.2
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
Process buildMingwCore cross_mingw_core
echo "** QT4 **"
cd $TOP
Process buildMingwQt4 cross_mingw_qt4 
#echo "** GTK **"
#cd $TOP
#Process buildGtk ../avidemux/gtk
echo "** Plugins **"
cd $TOP
Process buildMingwPlugins cross_mingw_plugins 
echo "** All done **"
cd $TOP
echo "** ALL DONE **"
