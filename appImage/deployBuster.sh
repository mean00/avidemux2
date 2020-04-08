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
cpyX86Optional()
{
        cp -t ../../opt/lib  /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1

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
cp avidemux3_qt5 avidemux3_portable
mkdir -p ../lib/qt5
mkdir -p ../lib/qt5/plugins
mkdir -p ../../opt/lib
# libz
#cpyRootx86Lib libz.so.1

# qt5
ldd avidemux3_portable | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5/ || fail qt5
ldd avidemux3_portable | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5 || fail icu
cp ${QT_HOME}/lib/libQt5DBus.so.5 ../lib/qt5 || fail QtDbus
cp ${QT_HOME}/lib/libQt5XcbQpa.so.5  ../lib/qt5 || fail QtXcb

#cpyRootx86Lib libdbus-1.so.3

cp -Rap -t ../lib/qt5/plugins ${QT_HOME}/plugins/platforms  || fail qtplugins
cp -Rap -t ../lib/qt5/plugins ${QT_HOME}/plugins/xcbglintegrations || fail qxcbglintegrations
# Support libs
for i in libffi.so.6  libsqlite3.so.0 libpng16.so.16 libgobject-2.0.so.0 libgthread-2.0.so.0 
do
        cpyX86 $i
done

cp -t ../lib /lib/x86_64-linux-gnu/libpcre.so.3 || fail pcre
cp -t ../lib /lib/x86_64-linux-gnu/libglib-2.0.so.0 || fail glib
#cp -t ../lib /lib/x86_64-linux-gnu/libjson.so.0 || fail json
cp -t ../lib /lib/x86_64-linux-gnu/libjson-c.so.3 || fail json
cpyRootx86Lib libudev.so.1

# audio plugins
for i in libfaad.so.2 libfaac.so.0 libmp3lame.so.0 libvorbis.so.0 libvorbisenc.so.2 libogg.so.0
	#libaften.so.0 
do
        cpyX86 $i
done

cpyX86 libopus.so.0 
# Audio device
cpyX86 libpulse-simple.so.0
cpyX86 libpulse.so.0
cpyX86 libfdk-aac.so.1
cp -Rap /usr/lib/x86_64-linux-gnu/pulseaudio ../lib/ || fail pulsecommon_folder
cp -t ../lib /usr/lib/x86_64-linux-gnu/pulseaudio/libpulsecommon-12.2.so || fail pulsecommon
 
# subtitles
for i in libfreetype.so.6 libfribidi.so.0 libfontconfig.so.1 
do 
        cpyX86Optional $i
done
cpyRootx86Lib libexpat.so.1
# Misc
for i in libFLAC.so.8 libbsd.so.0 libcap.so.2 libgcc_s.so.1 libgcrypt.so.20 liblz4.so.1 liblzma.so.5 libstdc++.so.6 libwrap.so.0 libz.so.1 libasyncns.so.0 libdbus-1.so.3 libdl.so.2 libdrm.so.2 libdrm_intel.so.1 libm.so.6 libsndfile.so.1 libdouble-conversion.so.1 libfontconfig.so.1 libfreetype.so.6 libharfbuzz.so.0
  do
     cpyRootx86Lib $i   
  done

# x264
#cpyLib libx264.so.152
# 
#cpyLib libx265.so.146
# Display lib
cpyX86 libvdpau.so.1
cpyX86 libXv.so.1
cpyX86 libva.so.2
cpyX86 libva-x11.so.2

cp -t ../lib /usr/lib/x86_64-linux-gnu/dri/nvidia_drv_video.so || fail nvidia_drv_video
cp -t ../lib /usr/lib/x86_64-linux-gnu/dri/i965_drv_video.so  || fail intel_drv_video
# patch
sed -i -e 's|/usr/lib/x86_64-linux-gnu/dri|././/././lib|g'   ../lib/libva.so.2
sed -i -e 's|/usr/lib/x86_64-linux-gnu/dri|././././././/./././././/./lib|g'  ../lib/libva.so.2

#intel, not sure it works

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

FT_PROBE_EXE_NAME="freetype_probe"
FT_PROBE_LOCATION="buildPluginsCommon/ADM_videoFilters6/ass"
if [ -e "${FT_PROBE_LOCATION}/${FT_PROBE_EXE_NAME}" ]; then
    cp "${FT_PROBE_LOCATION}/${FT_PROBE_EXE_NAME}" install
    chmod +x "install/${FT_PROBE_EXE_NAME}"
fi

cd $ORG
#AppImageAssistant  install $APP_NAME
# Patch desktop file
cp  install/avidemux.desktop /tmp
cat /tmp/avidemux.desktop | sed 's/avidemux.png/avidemux/g' > install/avidemux.desktop
appimagetool install

