#!/bin/bash
# look here https://github.com/probonopd/AppImages/blob/master/recipes/scribus/Recipe
# export QT_DEBUG_PLUGINS=1 
# export LD_DEBUG=amm LD_DEBUG_OUTPUT=/tmp/somefile

fail()
{
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        exit 1

}
cpyX86()
{
        cp -t ../lib  /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1

}
cpyX86Rename()
{
        cp -t ../lib/$2  /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1
}
cpyLib()
{
        cp -t ../lib  /usr/lib/$1 || fail copy_lib $i

}
cpyRootLib()
{
        cp -t ../lib  /lib/$1 || fail copy_lib $i

}
cpyRootx86Lib()
{
        cp -t ../lib  /lib/x86_64-linux-gnu/$1 || fail copy_lib $i

}



echo " ** Creating AppImage file **"

if [ "${QT_HOME}" = "" ] ; then
  echo "QT_HOME not set, cancelling"
  exit 1
fi
export LD_LIBRARY_PATH=$QT_HOME/lib:$LD_LIBRARY_PATH
export ORG=$PWD
export APP_NAME=app
rm -f $APP_NAME
cd install/usr/bin
mkdir -p ../lib/qt5
mkdir -p ../lib/qt5/plugins
# libz
#cpyRootx86Lib libz.so.1

# qt5
ldd avidemux3_qt5 | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5/ || fail qt5
ldd avidemux3_qt5 | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5 || fail icu
cp ${QT_HOME}/lib/libQt5DBus.so.5 ../lib/qt5 || failQtDbus
cp ${QT_HOME}/lib/libQt5XcbQpa.so.5  ../lib/qt5 || failQtXcb

#cpyRootx86Lib libdbus-1.so.3

cp -Rap -t ../lib/qt5/plugins ${QT_HOME}/plugins/platforms  || fail qtplugins
# Support libs
for i in libffi.so.5  libsqlite3.so.0 libpng12.so.0 libgobject-2.0.so.0 libgthread-2.0.so.0 
do
        cpyX86 $i
done

cp -t ../lib /lib/x86_64-linux-gnu/libpcre.so.3 || fail pcre
cp -t ../lib /lib/x86_64-linux-gnu/libglib-2.0.so.0 || fail glib
cp -t ../lib /lib/x86_64-linux-gnu/libjson.so.0 || fail json
# xcb
for i in libX11-xcb.so.1 libxcb-render-util.so.0 libxcb-glx.so.0 libxcb-render.so.0 libxcb.so.1 libxcb-image.so.0 libxcb-icccm.so.4 libxcb-sync.so.1 libxcb-xfixes.so.0 libxcb-shm.so.0 libxcb-randr.so.0 libxcb-shape.so.0 libxcb-keysyms.so.1 libxcb-xkb.so.1 libxcb-util.so.0 libxcb-dri2.so.0 libdrm.so.2
do 
	echo $i
  # cpyX86 $i
done
# Misc
#for i in libGL.so.1 libGLU.so.1 libsasl2.so.2 libEGL.so.1
#do
	#cpyX86 $i
#done
cpyRootx86Lib libudev.so.0
# audio plugins
for i in libfaad.so.2 libfaac.so.0 libmp3lame.so.0 libvorbis.so.0 libvorbisenc.so.2 libaften.so.0 libogg.so.0
do
        cpyX86 $i
done

cpyLib libopus.so.0 
# Audio device
cpyX86 libpulse-simple.so.0
cpyX86 libpulse.so.0
cp -Rap /usr/lib/x86_64-linux-gnu/pulseaudio ../lib/ || fail pulsecommon_folder
cp -t ../lib /usr/lib/x86_64-linux-gnu/pulseaudio/libpulsecommon-2.0.so || fail pulsecommon
 
# subtitles
for i in libfreetype.so.6 libfribidi.so.0 libfontconfig.so.1 
do 
        cpyX86 $i
done
cpyRootx86Lib libexpat.so.1
# x264
cpyLib libx264.so.148
cpyLib libx265.so.79
# Display lib
cpyX86 libvdpau.so.1
cpyX86 libXv.so.1
cpyLib libva.so.1
cpyLib libva-x11.so.1

cp -t ../lib /usr/lib/dri/nvidia_drv_video.so || fail nvidia_drv_video
# patch
sed -i -e 's|/usr/lib/dri|././/././lib|g'   ../lib/libva.so.1
sed -i -e 's|/usr/lib/x86_64-linux-gnu/dri|././././././/./././././/./lib|g'  ../lib/libva.so.1


# Fixup link
cd ../lib
cd ../bin
# Qt path
echo "[Paths]" > qt.conf
echo "Prefix=../lib/qt5" >> qt.conf


cd ..
#find . -type f -exec sed -i -e 's|/usr|../lib/|g' {} \; ; cd -
find . -type f -exec sed -i -e 's|/usr/lib/x86_64-linux-gnu|./././././././././././lib|g' {} \; ; cd ..
find . -type f -exec sed -i -e 's|/usr/lib|././/lib|g' {} \; ; cd ..
#
cd $ORG
cp appImage/AppRun install
cp appImage/avidemux.png install
cp appImage/avidemux.desktop install

cd $ORG
AppImageAssistant  install $APP_NAME

