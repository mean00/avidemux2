#!/bin/bash
mkdir -p install
rm -f install/*
echo "#* Preparing 7z svn build *#*"
echo "** Getting svn revision **"
export REV=`git log | grep branch_mean | head -1 | sed 's/^.*branch_mean.//g' | sed 's/ .*$//g'`
cp /mingw/bin/libfribidi.dll /mingw/Release/libfribidi-0.dll
cp /mingw/bin/libfreetype-6.dll /mingw/Release/
cp /mingw/lib/xvidcore.dll /mingw/Release/
cp /mingw/bin/libogg-0.dll /mingw/Release/
cp /mingw/bin/*vorbis*.dll /mingw/Release/
tar -xzlf ~/adm_fonts.tgz -C /mingw/Release/
#cp /mingw/bin/iconv.dll /mingw/Release/
echo "Revision : $REV"
echo "Creating NSIS Package"
mkdir -p install
rm -f install/*
makensis -DSVN_VERSION="${REV}" -DNSIDIR=$PWD avidemux_cross64.nsi
echo "Done"
