

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
        rm -Rf $BUILDDIR
        mkdir $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        cmake -DCMAKE_INSTALL_PREFIX=/usr $SOURCEDIR || fail cmake
        make -j 2 > /tmp/log$BUILDDIR || fail make
        sudo make install || fail install
        sudo chown -R `whoami` .
        fakeroot make package || fail package
}

export TOP=$PWD
sudo -v
echo "**BootStrapping avidemux **"
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
