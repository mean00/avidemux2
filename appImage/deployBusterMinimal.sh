#!/bin/bash
#############################################################################
# Populate appImage with a minimum set of required libraries, but leave out #
# conflicting libc and libGL stuff. This presumes that glibc 2.28 or later, #
# legacy libGL and PulseAudio are available on host.                        #
#############################################################################

# export QT_DEBUG_PLUGINS=1 
# export LD_DEBUG=all LD_DEBUG_OUTPUT=/tmp/somefile

fail()
{
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        echo "************************* $* FAILED"
        exit 1

}
cpyX86()
{
        cp -t ../lib  /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1

}
cpyX86Optional()
{
        cp -t ../../opt/lib  /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1

}
cpyX86Rename()
{
        cp -t ../lib/$2  /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1
}
cpyLib()
{
        cp -t ../lib  /usr/lib/$1 || fail copy_lib $i

}
cpyRootLib()
{
        cp -t ../lib  /lib/$1 || fail copy_lib $i

}
cpyRootx86Lib()
{
        cp -t ../lib  /lib/x86_64-linux-gnu/$1 || fail copy_lib $i

}



echo " ** Creating AppImage file **"

ORG=$PWD
RUNTIME="$1"
if [ ! -f "${RUNTIME}" ]; then
    echo "AppImage runtime \"${RUNTIME}\" not present or not a file, aborting."
    exit 1
fi
export APP_NAME="avidemuxLinux_GLIBC_2.28_amd64_$(date +%y%m%d_%H%M).app"
if [ -e ${APP_NAME} ]; then
    rm -f $APP_NAME || exit 1
fi
cd install/usr/bin || exit 1
mkdir -p ../lib/va || exit 1
mkdir -p ../lib/vdpau || exit 1
mkdir -p ../lib/qt5/plugins || exit 1
mkdir -p ../../opt/lib || exit 1

# qt5
ldd avidemux3_qt5 | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5/ || fail qt5
ldd avidemux3_qt5 | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt5 || fail icu
cp /usr/lib/x86_64-linux-gnu/libQt5DBus.so.5 ../lib/qt5 || fail QtDbus
cp /usr/lib/x86_64-linux-gnu/libQt5XcbQpa.so.5  ../lib/qt5 || fail QtXcb

cp -Rap -t ../lib/qt5/plugins /usr/lib/x86_64-linux-gnu/qt5/plugins/platforms  || fail qtplugins
cp -Rap -t ../lib/qt5/plugins /usr/lib/x86_64-linux-gnu/qt5/plugins/xcbglintegrations || fail qxcbglintegrations

cpyRootx86Lib libdouble-conversion.so.1
cpyX86 libxcb-xinerama.so.0

# various libs
# The bundled libva doesn't work, but allows Avidemux to run
# if no system libva is present.
LIBVA_LIBS="libpcre2-16.so.0 \
libva.so.2 \
libva-x11.so.2 \
libva-drm.so.2 \
libdrm_intel.so.1"

for i in ${LIBVA_LIBS}
do
        cpyX86Rename $i va
done

# Same for libvdpau.
cpyX86Rename libvdpau.so.1 vdpau

# Trying to use Buster's libGL on other hosts crashes Avidemux.
# We still need libGLU, not present by default on Ubuntu and Fedora.
DISPLAY_LIBS="libGLU.so.1 libXv.so.1"

AUDIO_PLUGINS="libfaac.so.0 \
libfdk-aac.so.1 \
libmad.so.0 \
libmp3lame.so.0 \
libtwolame.so.0 \
libvorbis.so.0 \
libvorbisenc.so.2 \
libogg.so.0"

VIDEO_PLUGINS="libx264.so.155 \
libx265.so.165 \
libvpx.so.5 \
libaom.so.0"

for i in ${DISPLAY_LIBS} ${AUDIO_PLUGINS} ${VIDEO_PLUGINS}
do
        cpyX86 $i
done

# subtitles
cpyX86 libass.so.9
for i in libfreetype.so.6 libfribidi.so.0 libfontconfig.so.1 libgraphite2.so.3
do
        cpyX86Optional $i
done

# Qt path
echo "[Paths]" > qt.conf
echo "Prefix=../lib/qt5" >> qt.conf

cd ..
find . -type f -exec sed -i -e 's|/usr/lib/x86_64-linux-gnu|./././././././././././lib|g' {} \; ; cd ..
find . -type f -exec sed -i -e 's|/usr/lib|././/lib|g' {} \; ; cd ..
#
cd $ORG
cp appImage/AppRunBuster install/AppRun
cp appImage/avidemux.png install
cp appImage/avidemux.desktop install

FT_PROBE_EXE_NAME="freetype_probe"
FT_PROBE_LOCATION="buildPluginsCommon/ADM_videoFilters6/ass"
if [ -e "${FT_PROBE_LOCATION}/${FT_PROBE_EXE_NAME}" ]; then
    cp "${FT_PROBE_LOCATION}/${FT_PROBE_EXE_NAME}" install
    chmod +x "install/${FT_PROBE_EXE_NAME}"
fi

cd $ORG
# Patch desktop file
cat install/avidemux.desktop | sed 's/avidemux.png/avidemux/g' > install/avidemux.desktop
if [ -f /tmp/myappimage.squashfs ]; then
    rm -f /tmp/myappimage.squashfs || exit 1
fi
mksquashfs install /tmp/myappimage.squashfs -root-owned -noappend || fail mksquashfs
cp "${RUNTIME}" "${APP_NAME}"
cat /tmp/myappimage.squashfs >> "${APP_NAME}" || fail "Appending squashfs to runtime"
chmod +x "${APP_NAME}"
echo "AppImage created as ${APP_NAME}"
