#!/bin/bash

fail()
{
        echo "** Failed at $1**"
        exit 1
}

Process()
{
        export BUILDDIR=$1
        export SOURCEDIR=$2
        echo "Building $BUILDDIR from $SOURCEDIR"
        rm -Rf ./$BUILDDIR
        mkdir $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        cmake -DCMAKE_INSTALL_PREFIX=/usr $SOURCEDIR || fail cmake
        make -j 2 > /tmp/log$BUILDDIR || fail make
        fakeroot make package DESTDIR=debPack || fail package
}

echo "**BootStrapping avidemux **"
export TOP=$PWD
echo "Top dir : $TOP"
echo "** CORE **"
cd $TOP
Process buildCore ../avidemux_core
echo "** QT4 **"
cd $TOP
Process buildQt4 ../avidemux/qt4
echo "** GTK **"
cd $TOP
Process buildGtk ../avidemux/gtk
echo "** Preparing debs **"
cd $TOP
rm -Rf debs
mkdir debs
find . -name "*.deb" | grep -vi cpa | xargs cp -t debs
echo "** debs directory ready **"
ls -l debs
echo "** ALL DONE **"
