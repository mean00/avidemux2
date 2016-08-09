#!/bin/sh
export ORG=$PWD
cd install/usr 
echo "** collecting ldd infos **"
export LD_LIBRARY_PATH=$PWD/lib:$PWD/lib/qt5
find . -type f -exec  ldd-recursive.pl -uniq  {} \; > /tmp/alldeps.txt
echo "** result **"
cat /tmp/alldeps.txt | grep -v avidemux2 | sort | uniq | grep -vi libx | grep -v wayland | grep -v nvidia

cd $ORG

