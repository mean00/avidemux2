#!/bin/bash
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


echo " ** Creating AppImage file **"

export ORG=$PWD
export APP_NAME=app
rm -f $APP_NAME
cd install/usr/bin
mkdir -p ../lib/qt5
mkdir -p ../lib/qt5/plugins
# libz
cp -t ../lib /lib/x86_64-linux-gnu/libz.so.1

# qt5
ldd avidemux3_qt5 | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5/
ldd avidemux3_qt5 | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5
cp /usr/lib/x86_64-linux-gnu/libQt5DBus.so.5 ../lib/qt5
cp -Rap -t ../lib/qt5/plugins /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms 
# Support libs
for i in libffi.so.5  libsqlite3.so.0 libpng12.so.0 libgobject-2.0.so.0 libgthread-2.0.so.0
do
        cpyX86 $i
done

cp -t ../lib /lib/x86_64-linux-gnu/libpcre.so.3 || fail pcre
cp -t ../lib /lib/x86_64-linux-gnu/libglib-2.0.so.0 || fail glib
cp -t ../lib /lib/x86_64-linux-gnu/libjson.so.0 || fail json

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
cp -t ../lib /lib/x86_64-linux-gnu/libexpat.so.1
# x264
cpyLib libx264.so.148
cpyLib libx265.so.79
# Display lib
cpyX86 libvdpau.so
cpyX86 libXv.so.1

# Fixup link
cd ../lib
cd ../bin
# Qt path
echo "[Paths]" > qt.conf
echo "Prefix=../lib/qt5" >> qt.conf


cd ..
#find . -type f -exec sed -i -e 's|/usr|../lib/|g' {} \; ; cd -
find . -type f -exec sed -i -e 's|/usr/lib/x86_64-linux-gnu|././/lib|g' {} \; ; cd ..
find . -type f -exec sed -i -e 's|/usr/lib|././/lib|g' {} \; ; cd ..

cd $ORG
AppImageAssistant  install $APP_NAME

