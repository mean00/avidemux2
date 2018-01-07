#!/bin/bash
t=`dirname $0`
export TOP_FOLDER="$t/../../"
echo "#* Preparing 7z svn build *#*"
echo "** Getting svn revision **"
#cp /mingw/bin/iconv.dll /mingw/Release/
echo "Revision : $REV"
echo "Creating NSIS Package"
makensis -DBINARY_FOLDER=/mingw/avidemux_32 -DDEV_FOLDER=/mingw_dev/mingw/Release -DSOURCE_FOLDER=$TOP_FOLDER -DAPI_VERSION=2.7 -DNSIDIR=$PWD avidemux_cross32Qt5.nsi
echo "Done"
