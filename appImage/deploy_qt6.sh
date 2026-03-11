#!/bin/bash
# look here https://github.com/probonopd/AppImages/blob/master/recipes/scribus/Recipe
# export QT_DEBUG_PLUGINS=1
fail() {
  echo "************************* $* FAILED"
  echo "************************* $* FAILED"
  echo "************************* $* FAILED"
  echo "************************* $* FAILED"
  echo "************************* $* FAILED"
  echo "************************* $* FAILED"
  echo "************************* $* FAILED"
  exit 1

}
set -x
export QT6_PLUGINS=/usr/lib/x86_64-linux-gnu/qt6/plugins/
# export LD_DEBUG=amm LD_DEBUG_OUTPUT=/tmp/somefile
export SOURCE=$1
export DEST=$2

echo "Installing $SOURCE -> $DEST"

rm -Rf $DEST
mkdir $DEST
cp -Rap $SOURCE/* $DEST/
mv $DEST/usr/lib/x86_64-linux-gnu/* $DEST/usr/lib
export H=$PWD
cd $DEST/usr/lib/x86_64-linux-gnu/
ln -s ../ADM_plugins6 .
cd $H
mv $DEST/usr/bin/avidemux3_qt6 $DEST/usr/bin/avidemux3_portable
#
#
#
mkdir -p $DEST/usr/plugins/platforms
cp -t ${DEST}/usr/plugins/platforms/ ${QT6_PLUGINS}/platforms/*wayland* ${QT6_PLUGINS}/platforms/*xcb*
cp -Rap -t ${DEST}/usr/plugins/ ${QT6_PLUGINS}/*integration*
#
#
cp -t ${DEST}/usr/lib /usr/lib/x86_64-linux-gnu/libwayland-client.so
cp -t ${DEST}/usr/lib /usr/lib/x86_64-linux-gnu/libwayland-cursor.so
cp -t ${DEST}/usr/lib /usr/lib/x86_64-linux-gnu/libwayland-egl.so
python3 appImage/checkDeps.py $DEST

exit 0
