export APP_NAME=app
rm -f $APP_NAME
cd install/usr/bin
mkdir -p ../lib/qt5
mkdir -p ../lib/qt5/plugins

ldd avidemux3_qt5 | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5/
ldd avidemux3_qt5 | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5
cp -Rap -t ../lib/qt5/plugins /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms 
# Dbus + glib + gobject+ffi+pcre
echo "[Paths]" > qt.conf
echo "Prefix=../lib/qt5" >> qt.conf

# Patch reloc
#cd ..
#find . -type f -exec sed -i -e 's|/usr|././|g' {} \; ; cd -
#cd bin
cd ../../..
AppImageAssistant  install $APP_NAME

