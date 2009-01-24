#!/bin/bash

# Manually generate project file so GTK sources can be excluded.
# Otherwise use: qmake -project -o avidemux.pro ..

echo "Finding HEADERS..."
echo "HEADERS = \\" > avidemux.pro
find ../avidemux -iname '*.h' -not -iwholename '*ADM_GTK*' -not -iwholename '*ADM_NONE*' -printf "%p \\\ \n" >> avidemux.pro
echo "" >> avidemux.pro

echo "Finding SOURCES..."
echo "SOURCES = \\" >> avidemux.pro
find ../avidemux -iname '*.cpp' -not -iwholename '*ADM_GTK*' -not -iwholename '*ADM_NONE*' -printf "%p \\\ \n" >> avidemux.pro
echo "" >> avidemux.pro

echo "Finding FORMS..."
echo "FORMS = \\" >> avidemux.pro
find ../avidemux -iname '*.ui' -not -iwholename '*ADM_GTK*' -not -iwholename '*ADM_NONE*' -printf "%p \\\ \n" >> avidemux.pro
echo "" >> avidemux.pro

echo "Finding TRANSLATIONS..."
echo "TRANSLATIONS = \\" >> avidemux.pro
find . -iname 'avidemux_*.ts' -printf "%p \\\ \n" >> avidemux.pro
echo "" >> avidemux.pro

echo "DONE."
