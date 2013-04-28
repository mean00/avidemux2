#!/bin/bash
# Manually generate project file so GTK sources can be excluded.
# Otherwise use: qmake -project -o avidemux.pro
echo "Finding HEADERS..."
echo "HEADERS = \\" > avidemux.pro
find .. -name "*.h*"  | grep -v build >> avidemux.pro
find ../../common -name "*.h" | grep -v build >> avidemux.pro
echo "" >> avidemux.pro
echo "Finding SOURCES..."
echo "SOURCES = \\" >> avidemux.pro
find .. -iname '*.cpp' -not -iwholename '*ADM_GTK*' -not -iwholename '*ADM_NONE*' -printf "\"%p\" \\\ \n" >> avidemux.pro
find ../../common -iname '*.cpp' -not -iwholename '*ADM_GTK*' -not -iwholename '*ADM_NONE*' -printf "\"%p\" \\\ \n" >> avidemux.pro
find ../../../avidemux_plugins -iname '*.cpp' -not -iwholename '*gtk*' -printf "\"%p\" \\\ \n" >> avidemux.pro
echo "" >> avidemux.pro
echo "Finding FORMS..."
echo "FORMS = \\" >> avidemux.pro
find ../ -iname '*.ui' -not -iwholename '*ADM_GTK*' -not -iwholename '*ADM_NONE*' -printf "\"%p\" \\\ \n" >> avidemux.pro
find ../../../avidemux_plugins -iname '*.ui' -not -iwholename '*gtk*' -printf "\"%p\" \\\ \n" >> avidemux.pro
echo "" >> avidemux.pro
echo "Finding TRANSLATIONS..."
echo "TRANSLATIONS = \\" >> avidemux.pro
find . -iname 'avidemux_*.ts' -printf "\"%p\" \\\ \n" >> avidemux.pro
#
lupdate-qt4 -pro avidemux.pro -ts avidemux_fr.ts
echo "DONE."
