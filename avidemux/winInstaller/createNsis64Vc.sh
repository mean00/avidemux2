#!/bin/bash
set -x
export REV="0"
export NSIS="/c/Program Files (x86)/NSIS/makensis.exe"
export TOP_FOLDER="/z/vs/"
export INSTALLER_FOLDER="/z/vs/avidemux/winInstaller"
# SetCompressor /SOLID lzma
mkdir -p install
rm -f install/*
# Except 
cat ${INSTALLER_FOLDER}/avidemux_crossQt5.nsi  
cat ${INSTALLER_FOLDER}/avidemux_crossQt5.nsi  | sed 's/\//\\/g'  | sed 's/\\SOLID/\/SOLID/g' > ${INSTALLER_FOLDER}/avidemux_nativeQt5Common.nsi
#cp  ${INSTALLER_FOLDER}/avidemux_nativeVcQt5.nsi avidemux_nativeVcQt5r.nsi 
cp  ${INSTALLER_FOLDER}/avidemux_nativeQt5Tail.nsi .
#cp  ${INSTALLER_FOLDER}/*.nsh .
#cp -r ${INSTALLER_FOLDER}/plugin .


echo "Creating NSIS Package"
mkdir -p install
rm -f install/*
#touch "Build Info.txt"
#touch "ChangeLog.html"
#touch "change.css"
"${NSIS}"  -DAPI_VERSION=2.7   -DNSIDIR=${INSTALLER_FOLDER} -DSOURCE_FOLDER=${TOP_FOLDER}  -DDEV_FOLDER="/z/runVs_dev" -DBINARY_DIR="/z/runVs" ${INSTALLER_FOLDER}/avidemux_nativeVcQt5.nsi -V4
echo "Done"
