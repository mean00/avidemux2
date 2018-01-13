#!/bin/bash
BITS=$1
echo " Creating cross Nsis package"
if [ "$BITS" == "32" ] || [  "$BITS" == "64" ]  
then
    echo "Variant $BITS bits"
else
    echo "Failed : 32 or 64" 
    exit 1
fi
t=`dirname $0`
export TOP_FOLDER="$t/../../"
echo "Revision : $REV"
echo "Version  : $Bits"
echo "Creating NSIS Package"
makensis -DEXTRA="" -DBINARY_FOLDER=/mingw/avidemux_${BITS} -DDEV_FOLDER=/mingw_dev/mingw/Release -DSOURCE_FOLDER=$TOP_FOLDER -DAPI_VERSION=2.7 -DNSIDIR=$PWD avidemux_cross${BITS}Qt5.nsi
echo "Done"
