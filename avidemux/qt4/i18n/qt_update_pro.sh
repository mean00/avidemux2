#!/bin/bash
# Manually generate project file so GTK sources can be excluded.
# Otherwise use: qmake -project -o avidemux.pro2
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
echo "Finding HEADERS..."
echo "HEADERS = \\" > avidemux.pro2
findHeader ..
findHeader ../../common
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

echo "Finding SOURCES..."
echo "SOURCES = \\" >> avidemux.pro2
findSource ..
findSource ../../common
findSource ../../../avidemux_plugins
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2
echo "Finding FORMS..."
echo "FORMS = \\" >> avidemux.pro2
findUI ..
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

echo "Finding TRANSLATIONS..."
echo "TRANSLATIONS = \\" >> avidemux.pro2
find . -iname 'avidemux_*.ts' -printf "\"%p\" \\\ \n" >> avidemux.pro2
echo "" >> avidemux.pro2
echo "" >> avidemux.pro2

cat avidemux.pro2 | sed 's/"//g' | grep -v build | grep -v cmake  | grep -v gtk >     avidemux.pro
#
lupdate-qt4 -pro avidemux.pro 
echo "DONE."
