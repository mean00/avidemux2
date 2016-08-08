#!/bin/bash
export ORG=$PWD
export APP_NAME=app
rm -f $APP_NAME
cd install/usr/bin
mkdir -p ../lib/qt5
mkdir -p ../lib/qt5/plugins

# qt5
ldd avidemux3_qt5 | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5/
ldd avidemux3_qt5 | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5
cp /usr/lib/x86_64-linux-gnu/libQt5DBus.so.5 ../lib/qt5
cp -Rap -t ../lib/qt5/plugins /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms 
# Support libs
for i in libffi.so.5 libglib-2.0.so libgobject-2.0.so libpcre.so
do
        cp -t ../lib/ /usr/lib/x86_64-linux-gnu/$i
done


# Qt path
echo "[Paths]" > qt.conf
echo "Prefix=../lib/qt5" >> qt.conf


cd ..
#find . -type f -exec sed -i -e 's|/usr|../lib/|g' {} \; ; cd -
find . -type f -exec sed -i -e 's|/usr/lib|././/lib|g' {} \; ; cd ..

cd $ORG
AppImageAssistant  install $APP_NAME

