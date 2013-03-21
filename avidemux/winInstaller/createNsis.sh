#!/bin/bash
echo "#* Preparing 7z svn build *#*"
echo "** Getting svn revision **"
export REV=`git log | grep branch_mean | head -1 | sed 's/^.*branch_mean.//g' | sed 's/ .*$//g'`
echo "Revision : $REV"
echo "Creating NSIS Package"
makensis -DSVN_VERSION="${REV}_8" -DNSIDIR=$PWD avidemux_cross.nsi
echo "Done"
