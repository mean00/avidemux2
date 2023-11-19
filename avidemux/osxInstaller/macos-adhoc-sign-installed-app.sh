#!/bin/sh
BUNDLE_CONTENT="/Applications/Avidemux_2.8.2.app/Contents"
if [ ! -d ${BUNDLE_CONTENT} ]
    then echo "Error: directory ${BUNDLE_CONTENT} doesn't exist." && exit 1
fi
# Qt frameworks
for fw in QtCore QtDBus QtGui QtNetwork QtOpenGL QtOpenGLWidgets QtWidgets
    do codesign --force -s - ${BUNDLE_CONTENT}/Frameworks/${fw}.framework/Versions/A/${fw}
done
# Qt plugins
for folder in PlugIns/platforms PlugIns/styles
    do codesign --force -s - ${BUNDLE_CONTENT}/${folder}/*.dylib
done
# Other frameworks
for lib in ${BUNDLE_CONTENT}/Frameworks/*.dylib
    do codesign --force -s - ${lib}
done
# Avidemux plugins
for folder in audioDecoder audioDevices audioEncoders demuxers muxers scriptEngines videoDecoders videoEncoders videoEncoders/qt6 videoFilters videoFilters/cli videoFilters/qt6
    do codesign --force -s - ${BUNDLE_CONTENT}/MacOS/ADM_plugins6/${folder}/*.dylib
done
# Executables
for exe in Avidemux2.8 avidemux_cli avidemux_jobs
    do codesign --force -s - ${BUNDLE_CONTENT}/MacOS/${exe}
done
exit 0