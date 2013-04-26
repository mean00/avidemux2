#!/bin/bash
echo "#* Preparing 7z svn build *#*"
echo "** Getting svn revision **"
export REV=`git log | grep branch_mean | head -1 | sed 's/^.*branch_mean.//g' | sed 's/ .*$//g'`
tar -xzlf ~/adm_fonts.tgz -C /mingw/Release/
echo "Revision : $REV"
echo "Creating NSIS Package"
mkdir -p install
rm -f install/*
git log  | grep -v "^commi" | grep -v "^Author" | grep -v "^Date" | grep -v "git-svn-id" | sed '/^[ \t]*$/d'  | head -15 > last_changes.txt
makensis -DSVN_VERSION="${REV}" -DNSIDIR=$PWD avidemux_cross.nsi
echo "Done"
