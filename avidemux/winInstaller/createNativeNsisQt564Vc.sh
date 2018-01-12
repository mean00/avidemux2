#!/bin/bash
set -x
export REV="0"
export NSIS="/c/Program Files (x86)/NSIS/makensis.exe"
export TOP_FOLDER="$1"
export INSTALLER_FOLDER="${TOP_FOLDER}/avidemux/winInstaller"
# SetCompressor /SOLID lzma
mkdir -p install
rm -f install/*
# Except 
export tmp="/tmp/tmp.$$"
export tmp2="/tmp/tmp2.$$"
cat ${INSTALLER_FOLDER}/avidemux_crossQt5.nsi  
cat ${INSTALLER_FOLDER}/avidemux_crossQt5.nsi  | sed 's/\//\\/g'  | sed 's/\\SOLID/\/SOLID/g' | sed 's/dll\.a/lib/g' > $tmp
cat $tmp | sed 's/^.*autoScripts.*$//g' | sed 's/^.*etc.fonts.*$//g' > $tmp2
cat $tmp2 | sed 's/^.*avsproxy.exe.*$//g' | sed 's/^.*vs[pP]roxy.*.exe.*$//g' > ${INSTALLER_FOLDER}/avidemux_nativeQt5Common.nsi
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
"${NSIS}"  -DAPI_VERSION=2.7  -DEXTRA=" VC++"  -DNSIDIR=${INSTALLER_FOLDER} -DSOURCE_FOLDER=${TOP_FOLDER}  -DDEV_FOLDER="$2" -DBINARY_FOLDER="$2" ${INSTALLER_FOLDER}/avidemux_nativeVcQt5.nsi -V4
echo "Done"
