#!/bin/bash
# ** Put your config here **
export QT_SELECT=5
export PATH=$PATH:/mingw/bin
export CROSS_PREFIX=i686-w64-mingw32
export SDLDIR=/mingw
export MINGW=/mingw
export MINGWDEV=/mingw_dev
export QT_HOME=/mingw/Qt/current
#export CFLAGS="-fpermissive"
export O_PARAL="-j 2"
export TOOLCHAIN_LOCATION=/mingw
export CROSS_C_COMPILER=clang
export CROSS_CXX_COMPILER=clang++
export CFLAGS="-mstackrealign -mstack-alignment=16 "
export CXXFLAGS="${CFLAGS}"


rebuild=0

# ** Put your config here **

fail()
{
        echo "** Failed at $1**"
        exit 1
}
usage()
{
        echo "Bootstrap avidemux 2.6:"
        echo "***********************"
        echo "  --rebuild         : Preserve existing build directories"
      
}

Process()
{
        export BUILDDIR=$1
        export SCRIPT=$2
        export EXTRA=$3
        echo "Building $BUILDDIR from $SOURCEDIR (Extra=$EXTRA)"
        if [ "x$rebuild" != "x1" ] ; then
                rm -Rf ./$BUILDDIR
        fi
        mkdir -p $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        sh $TOP/foreignBuilds/$SCRIPT $EXTRA || fail cmake
        make  $PARAL VERBOSE=1 || fail make
        make install || fail make_install
        # Only install  component=dev for dev package
	DESTDIR=${MINGWDEV} cmake -DCOMPONENT=dev -P cmake_install.cmake || fail make_install_dev
}

echo "**BootStrapping avidemux -cross Win32 clang version **"

while [ $# != 0 ] ;do
        case "$1" in
         -h|--help)
             usage
             exit 1
             ;;
           --rebuild)
                rebuild=1
             ;;
         *)
                echo "unknown parameter $1"
                usage
                exit 1
                ;;
         esac
     shift
done


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
