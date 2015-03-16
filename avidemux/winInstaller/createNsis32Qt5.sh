#!/bin/bash
mkdir -p install
rm -f install/*
echo "#* Preparing 7z svn build *#*"
echo "** Getting svn revision **"
export REV=`git log | grep branch_mean | head -1 | sed 's/^.*branch_mean.//g' | sed 's/ .*$//g'`
export REV="00"
#cp /mingw/bin/iconv.dll /mingw/Release/
echo "Revision : $REV"
echo "Creating NSIS Package"
mkdir -p install
rm -f install/*
makensis -DSVN_VERSION="${REV}" -DNSIDIR=$PWD avidemux_cross32Qt5.nsi
echo "Done"
