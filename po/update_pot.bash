#!/bin/bash
cd .. 
echo "Finding HEADERS..."
find ./avidemux -iname '*.h' -not -iwholename '*ADM_QT4*' > po/POTFILES.in

echo "Finding SOURCES..."
find ./avidemux -iname '*.cpp' -not -iwholename '*ADM_QT4*' >> po/POTFILES.in

cd po
echo "Generating pot file..."
xgettext --keyword=QT_TR_NOOP --debug --from-code=utf-8 -C -D .. -p . -f POTFILES.in -o avidemux.pot
echo "DONE."
