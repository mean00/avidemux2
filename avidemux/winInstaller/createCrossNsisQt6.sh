#!/bin/bash
BITS="64"
echo "Creating NSIS package for cross-compiled Qt6 Avidemux"
t=`dirname $0`
export TOP_FOLDER="$t/../../"
echo "Revision : $REV"
echo "Version  : $BITS"
echo "Creating NSIS Package"
makensis \
-DEXTRA="" \
-DBINARY_FOLDER=/mingw/avidemux_${BITS} \
-DDEV_FOLDER=/mingw_dev/mingw/Release \
-DSOURCE_FOLDER=$TOP_FOLDER \
-DAPI_VERSION=2.8 \
-DNSIDIR=$PWD \
-DGOT_AVISYNTH=1 \
avidemux_cross${BITS}Qt6.nsi
echo "Done"
