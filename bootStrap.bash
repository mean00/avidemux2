#!/bin/bash
# Bootstrapper to semi-automatically build avidemux deb/rpm from source
# (c) Mean 2009
#
packages_ext=""
do_core=1
do_cli=0
do_gtk=0
do_qt4=1
do_plugins=1
debug=0
export O_PARAL="-j 2"
fail()
{
        echo "** Failed at $1**"
        exit 1
}

Process()
{
        export BUILDDIR=$1
        export SOURCEDIR=$2
        export EXTRA=$3
        export DEBUG=""
        BUILDER="Unix Makefiles"
        if [ "x$debug" = "x1" ] ; then 
                DEBUG="-DVERBOSE=1 -DCMAKE_BUILD_TYPE=Debug  "
                BUILDDIR="${BUILDDIR}_debug"
                BUILDER="CodeBlocks - Unix Makefiles"
        fi
	FAKEROOT=" -DFAKEROOT=$FAKEROOT_DIR "
        echo "Building $BUILDDIR from $SOURCEDIR with EXTRA=<$EXTRA>, DEBUG=<$DEBUG>"
        rm -Rf ./$BUILDDIR
        mkdir $BUILDDIR || fail mkdir
        cd $BUILDDIR 
        cmake $PKG $FAKEROOT -DCMAKE_EDIT_COMMAND=vim -DAVIDEMUX_SOURCE_DIR=$TOP -DCMAKE_INSTALL_PREFIX=/usr $EXTRA $DEBUG -G "$BUILDER" $SOURCEDIR || fail cmakeZ
        make  $PARAL >& /tmp/log$BUILDDIR || fail "make, result in /tmp/log$BUILDDIR"
	if  [ "x$PKG" != "x" ] ; then
          $FAKEROOT_COMMAND make package DESTDIR=$FAKEROOT_DIR/tmp || fail package
	fi
    	make install DESTDIR=$FAKEROOT_DIR
}
printModule()
{
        value=$1
        name=$2
        if [ "x$value" = "x1" ]; then echo "    $name will be built"
        else echo "     $name will be skipped"
        fi
        

}
config()
{
        echo "Build configuration :"
        echo "******************* :"
        echo "Build type :"
        if [ "x$debug" = "x1" ] ; then echo   "Debug build"
        else echo   "Release build"
        fi
        printModule $do_core Core
        printModule $do_gtk Gtk
        printModule $do_qt4 Qt4
        printModule $do_cli Cli
        printModule $do_plugins Plugins
}
usage()
{
        echo "Bootstrap avidemux 2.6:"
        echo "***********************"
        echo "  --help            : Print usage"
        echo "  --rpm             : Build rpm packages"
        echo "  --deb             : Build deb packages"
        echo "  --tgz             : Build tgz packages"
        echo "  --debug           : Switch debugging on"
        echo "  --with-core       : Build core"
        echo "  --without-core    : Dont build core"
        echo "  --with-cli        : Build cli"
        echo "  --without-cli     : Dont build cli"
        echo "  --with-gtk        : Build gtk"
        echo "  --without-gtk     : Dont build gtk"
        echo "  --with-core       : Build core"
        echo "  --without-qt4     : Dont build qt4"
        echo "  --with-plugins    : Build plugins"
        echo "  --without-plugins : Dont build plugins"
	echo "The end result will be in the install folder. You can then copy it to / or whatever"
        config 

}
#

export FAKEROOT_COMMAND="fakeroot"
CMAKE_VERSION=`cmake --version | sed "s/^.* 2\.\([0-9]*\.[0-9]*\).*/2\.\1/g"`
echo "CMAKE Version : $CMAKE_VERSION"
case "$CMAKE_VERSION" in
         2.8.8|2.8.7|2.8.9)
                echo "Cmake version >=2.8.7 doesnt need fakeroot"
                export FAKEROOT_COMMAND=""
        ;;
esac
# Could probably do it with getopts...
while [ $# != 0 ] ;do
        case "$1" in
         -h|--help)
             usage
             exit 1
             ;;
         --debug)
                debug=1
                ;;
         --tgz)
                packages_ext=tar.gz
                PKG="$PKG -DAVIDEMUX_PACKAGER=tgz"
                ;;
         --deb)
                packages_ext=deb
                PKG="$PKG -DAVIDEMUX_PACKAGER=deb"
                ;;
         --rpm)
                packages_ext=rpm
                PKG="$PKG -DAVIDEMUX_PACKAGER=rpm"
                ;;
         --without-qt4)
                do_qt4=0
             ;;
         --without-cli)
                do_cli=0
             ;;
         --without-gtk)
                do_gtk=0
             ;;
         --without-plugins)
                do_plugins=0
             ;;
         --without-core)
                do_core=0
             ;;
         --with-qt4)
                do_qt4=1
             ;;
         --with-cli)
                do_cli=1
             ;;
         --with-gtk)
                do_gtk=1
             ;;
         --with-plugins)
                do_plugins=1
             ;;
         --with-core)
                do_core=1
             ;;
        *)
                echo "unknown parameter $1"
                usage
                exit 1
                ;;
     esac
     shift
done
config
echo "**BootStrapping avidemux **"

export TOP=$PWD
export POSTFIX=""
export FAKEROOT_DIR=$PWD/install
echo "Top dir : $TOP"
echo "Fake installation directory=$FAKEROOT_DIR"
if [ "x$debug" = "x1" ] ; then echo   
POSTFIX="_debug"
fi
if [ "x$packages_ext" = "x" ]; then 
	echo ""	
else
	rm -Rf $FAKEROOT_DIR
	mkdir -p $FAKEROOT_DIR
fi

if [ "x$do_core" = "x1" ] ; then 
        echo "** CORE **"
        cd $TOP
        export PARAL=""
        Process buildCore ../avidemux_core
        echo " Installing core"
        cd $TOP/buildCore${POSTFIX} 
fi
export PARAL="$O_PARAL"
if [ "x$do_qt4" = "x1" ] ; then 
        echo "** QT4 **"
        cd $TOP
        Process buildQt4 ../avidemux/qt4
        echo " Installing Qt4"
        cd $TOP/buildQt4${POSTFIX} 
fi
if [ "x$do_cli" = "x1" ] ; then 
        echo "** CLI **"
        cd $TOP
        Process buildCli ../avidemux/cli 
        echo " Installing cli"
        cd $TOP/buildCli${POSTFIX}  
fi
if [ "x$do_gtk" = "x1" ] ; then 
        echo "** GTK **"
        cd $TOP
        Process buildGtk ../avidemux/gtk 
        echo " Installing Gtk"
        cd $TOP/buildGtk${POSTFIX} 
fi
if [ "x$do_plugins" = "x1" ] ; then 
        echo "** Plugins **"
        cd $TOP
        Process buildPluginsCommon ../avidemux_plugins -DPLUGIN_UI=COMMON
fi
if [ "x$do_plugins" = "x1" -a "x$do_qt4" = "x1" ] ; then 
        echo "** Plugins Qt4 **"
        cd $TOP
        Process buildPluginsQt4 ../avidemux_plugins -DPLUGIN_UI=QT4
fi
if [ "x$do_plugins" = "x1" -a "x$do_gtk" = "x1" ] ; then 
        echo "** Plugins Gtk **"
        cd $TOP
        Process buildPluginsGtk ../avidemux_plugins -DPLUGIN_UI=GTK
fi
if [ "x$do_plugins" = "x1" -a "x$do_cli" = "x1" ] ; then 
        echo "** Plugins CLI **"
        cd $TOP
        Process buildPluginsCLI ../avidemux_plugins -DPLUGIN_UI=CLI
fi

echo "** Preparing debs **"
cd $TOP
if [ "x$packages_ext" = "x" ]; then 
        echo "No packaging"
else
        echo "Preparing packages"
        rm -Rf debs
        mkdir debs
        find . -name "*.$packages_ext" | grep -vi cpa | xargs cp -t debs
        echo "** debs directory ready **"
        ls -l debs
fi
echo "** ALL DONE **"
if [ "x$packages_ext" = "x" ]; then 
    echo "** Copy the $FAKEROOT_DIR folder to your favorite location, i.e. sudo cp -R install/usr/* /usr/ **"
else
    echo "** The installable packages are in the debs folder **"
fi
