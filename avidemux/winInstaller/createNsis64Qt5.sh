#!/bin/bash
mkdir -p install
rm -f install/*
sh genlog.sh
echo "#* Preparing 7z svn build *#*"
echo "** Getting svn revision **"
#cp /mingw/bin/iconv.dll /mingw/Release/
echo "Revision : $REV"
echo "Creating NSIS Package"
mkdir -p install
rm -f install/*
makensis  -DNSIDIR=$PWD avidemux_cross64Qt5.nsi
echo "Done"
