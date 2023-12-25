#!/bin/sh
fail()
{
    echo "** $1 **"
    exit 1
}
sign()
{
    codesign --force -s - $1 || fail "Error ad-hoc signing $1"
}

BUNDLE_CONTENT="/Applications/Avidemux-2.8.2.app/Contents"

if [ ! -d ${BUNDLE_CONTENT} ]
then
    fail "Error: directory ${BUNDLE_CONTENT} doesn't exist."
fi
#
# Qt frameworks
#
for fw in \
QtCore \
QtDBus \
QtGui \
QtNetwork \
QtOpenGL \
QtOpenGLWidgets \
QtWidgets
do
    sign ${BUNDLE_CONTENT}/Frameworks/${fw}.framework/Versions/A/${fw}
done
#
# Qt plugins
#
for folder in \
platforms \
styles
do
    for lib in ${BUNDLE_CONTENT}/PlugIns/${folder}/*.dylib
    do
        sign ${lib}
    done
done
#
# Other frameworks
#
for lib in ${BUNDLE_CONTENT}/Frameworks/*.dylib
do
    sign ${lib}
done
#
# Avidemux plugins
#
PLUGDIR=${BUNDLE_CONTENT}/MacOS/ADM_plugins6
for folder in \
audioDecoder \
audioDevices \
audioEncoders \
demuxers \
muxers \
scriptEngines \
videoDecoders \
videoEncoders \
videoEncoders/qt6 \
videoFilters \
videoFilters/cli \
videoFilters/qt6
do
    for lib in ${PLUGDIR}/${folder}/*.dylib
    do
        sign ${lib}
    done
done
#
# Do JSON encoder presets, tinyPy scripts & Co. need a signature too?
#
for textfile in \
${PLUGDIR}/autoScripts/*.py \
${PLUGDIR}/autoScripts/lib/*.py \
${PLUGDIR}/pluginSettings/x264/3/*.json \
${PLUGDIR}/shaderDemo/1/*.shader
do
    sign ${textfile}
done
#
# Executables
#
for exe in \
avidemux_cli \
avidemux_jobs
do
    sign ${BUNDLE_CONTENT}/MacOS/${exe}
done
#
# Re-signing the main Avidemux executable is expected to return an error, the app should work nevertheless.
#
codesign --force -s - ${BUNDLE_CONTENT}/MacOS/Avidemux2.8
echo "** Bundle format warning, if issued, should be ignored. **"
echo "** All done. **"
exit 0