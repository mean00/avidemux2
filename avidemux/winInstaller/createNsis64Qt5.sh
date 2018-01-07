#!/bin/bash
t=`dirname $0`
export TOP_FOLDER="$t/../../"
echo "** Getting svn revision **"
echo "Revision : $REV"
echo "Creating NSIS Package (64)"
makensis -DBINARY_FOLDER=/mingw/avidemux_64 -DDEV_FOLDER=/mingw_dev/mingw/Release -DSOURCE_FOLDER=$TOP_FOLDER -DAPI_VERSION=2.7 -DNSIDIR=$PWD avidemux_cross64Qt5.nsi
echo "Done"
