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
if [ -e /etc/redhat-release ]; then
  export LIBRARY_DIR=/usr/lib64
else
  export LIBRARY_DIR=/usr/lib/x86_64-linux-gnu
fi
export QT6_PLUGINS=$LIBRARY_DIR/qt6/plugins
# export LD_DEBUG=amm LD_DEBUG_OUTPUT=/tmp/somefile
export SOURCE=$1
export DEST=$2

echo "Installing ${SOURCE} -> ${DEST}"

rm -Rf ${DEST} || fail "Removing destination directory"
mkdir ${DEST}
cp -Rap ${SOURCE}/* ${DEST}/

if [ -e /etc/redhat-release ]; then
  pushd ${DEST}/usr && mv lib64 lib && ln -sf lib lib64
  popd
else
  mkdir -p ${DEST}/usr/lib && mv ${DEST}/${LIBRARY_DIR}/* ${DEST}/usr/lib/
  pushd ${DEST}/${LIBRARY_DIR} && ln -s ../ADM_plugins6 .
  popd
fi
mv ${DEST}/usr/bin/avidemux3_qt6 ${DEST}/usr/bin/avidemux3_portable
mv ${DEST}/usr/bin/avidemux3_jobs_qt6 ${DEST}/usr/bin/avidemux3_jobs_portable
#
#
#
mkdir -p ${DEST}/usr/plugins/platforms
cp -t ${DEST}/usr/plugins/platforms/ ${QT6_PLUGINS}/platforms/*wayland* ${QT6_PLUGINS}/platforms/*xcb*
cp -Rap -t ${DEST}/usr/plugins/ ${QT6_PLUGINS}/*integration*
cp -Rap -t ${DEST}/usr/plugins ${QT6_PLUGINS}/wayland-decoration-client
#
#
if [ -e /etc/redhat-release ]; then
  cp -t ${DEST}/usr/lib64 -H ${LIBRARY_DIR}/libwayland-client.so.0
  cp -t ${DEST}/usr/lib64 -H ${LIBRARY_DIR}/libwayland-cursor.so.0
  cp -t ${DEST}/usr/lib64 -H ${LIBRARY_DIR}/libwayland-egl.so.1
else
  cp -t ${DEST}/usr/lib ${LIBRARY_DIR}/libwayland-client.so
  cp -t ${DEST}/usr/lib ${LIBRARY_DIR}/libwayland-cursor.so
  cp -t ${DEST}/usr/lib ${LIBRARY_DIR}/libwayland-egl.so
fi
python3 appImage/checkDeps.py $DEST

exit 0
