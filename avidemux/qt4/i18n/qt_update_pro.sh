#!/bin/bash
# Manually generate project file so GTK sources can be excluded.
# Otherwise use: qmake -project -o avidemux.pro2
export QT_SELECT=5

function findHeader()
{
find "$1" -name "*.h*"  -printf "\"%p\" \\\ \n"   >> avidemux.pro2

}
function findSource()
{
find "$1" -name "*.c*"  -printf "\"%p\" \\\ \n"   >> avidemux.pro2

}
function findUI()
{
find "$1" -name "*.ui"  -printf "\"%p\" \\\ \n"   >> avidemux.pro2

}
#*******************************
export PRODIR=$PWD
cd ../../..
echo "Finding HEADERS..."
echo "HEADERS = \\" > avidemux.pro2
findHeader avidemux_core 
findHeader avidemux/qt4
findHeader avidemux/common
findHeader avidemux_plugins/
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

echo "Finding SOURCES..."
echo "SOURCES = \\" >> avidemux.pro2
findSource avidemux_core 
findSource avidemux/qt4
findSource avidemux/common
findSource avidemux_plugins/
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

echo "Finding FORMS..."
echo "FORMS = \\" >> avidemux.pro2
findUI avidemux_plugins/
findUI avidemux/qt4/
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

echo "Finding TRANSLATIONS..."
echo "TRANSLATIONS = \\" >> avidemux.pro2
cd $PRODIR
find . -iname 'avidemux_*.ts' -printf "\"%p\" \\\ \n" >> avidemux.pro2
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

cat avidemux.pro2 | sed 's/"//g' | grep -v "ADM_videoFilters\/" | grep -v qtScript | grep -v html |  grep -v build | grep -v cmake  | grep -v gtk | grep -v "cli\/" | grep -vi spidermon>     avidemux.pro
#
lupdate -pro avidemux.pro 
echo "DONE."
