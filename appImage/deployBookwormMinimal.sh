#!/bin/bash
#############################################################################
# Populate appImage with a minimum set of required libraries, but leave out #
# conflicting libc and libGL stuff. This presumes that glibc 2.36 or later, #
# legacy libGL and PulseAudio are available on host.                        #
#############################################################################

# export QT_DEBUG_PLUGINS=1 
# export LD_DEBUG=all LD_DEBUG_OUTPUT=/tmp/somefile

fail()
{
        echo "******** $* FAILED ********"
        exit 1

}
cpyX86()
{
        cp -t ../lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1
}
cpyX86Optional()
{
        cp -t ../../opt/lib /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1
}
cpyX86Rename()
{
        cp -t ../lib/$2 /usr/lib/x86_64-linux-gnu/$1 || fail copy_x86lib $1
}
cpyLocal()
{
        cp -t ../lib /usr/local/lib/$1 || fail copy_local_lib $i
}

echo " ** Creating AppImage file **"

APPIMAGE_SCRIPT_DIR=$(cd $(dirname "$0") && pwd)
RUNTIME="$1"
if [ ! -f "${RUNTIME}" ]; then
    echo "AppImage runtime \"${RUNTIME}\" not present or not a file, aborting."
    exit 1
fi
export APP_NAME="avidemuxLinux_GLIBC_2.36_amd64_$(date +%y%m%d_%H%M).app"
if [ -e "$APP_NAME" ]; then
    rm -f "$APP_NAME" || exit 1
fi
pushd install/usr/bin > /dev/null || exit 1
mkdir -p ../lib/va || exit 1
mkdir -p ../lib/vdpau || exit 1
mkdir -p ../lib/qt6/plugins || exit 1
mkdir -p ../../opt/lib || exit 1

# Qt6
ldd avidemux3_qt6 | grep libQ | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt6/ || fail qt6
ldd avidemux3_qt6 | grep icu | sed 's/^.*=>//g' | sed 's/ (.*$//g' | xargs cp -t ../lib/qt6 || fail icu
cp /usr/lib/x86_64-linux-gnu/libQt6DBus.so.6 ../lib/qt6 || fail QtDbus
cp /usr/lib/x86_64-linux-gnu/libQt6XcbQpa.so.6 ../lib/qt6 || fail QtXcb

cp -Rap -t ../lib/qt6/plugins /usr/lib/x86_64-linux-gnu/qt6/plugins/platforms || fail qtplugins
cp -Rap -t ../lib/qt6/plugins /usr/lib/x86_64-linux-gnu/qt6/plugins/platformthemes || fail qtplatformthemes
cp -Rap -t ../lib/qt6/plugins /usr/lib/x86_64-linux-gnu/qt6/plugins/xcbglintegrations || fail qxcbglintegrations

cpyX86 libdouble-conversion.so.3
cpyX86 libmd4c.so.0
cpyX86 libb2.so.1
cpyX86 libgomp.so.1

# various libs
# The bundled libva doesn't work, but allows Avidemux to run
# if no system libva is present.
LIBVA_LIBS="libpcre2-16.so.0 \
libva.so.2 \
libva-x11.so.2 \
libva-drm.so.2 \
libdrm_intel.so.1 \
libdrm_amdgpu.so.1 \
libdrm_radeon.so.1"

for i in ${LIBVA_LIBS}
do
        cpyX86Rename $i va
done

# Same for libvdpau.
cpyX86Rename libvdpau.so.1 vdpau

# Trying to use Bookworm's libGL on other hosts crashes Avidemux.
# We still need libGLU, not present by default on Ubuntu and Fedora.
DISPLAY_LIBS="libGLU.so.1 libXv.so.1"

AUDIO_PLUGINS="libfaac.so.0 \
libfdk-aac.so.2 \
libmad.so.0 \
libmp3lame.so.0 \
libopus.so.0 \
libtwolame.so.0 \
libvorbis.so.0 \
libvorbisenc.so.2 \
libogg.so.0"

VIDEO_PLUGINS="libx264.so.164 \
libx265.so.199 \
libxvidcore.so.4 \
libvpx.so.7"

for i in ${DISPLAY_LIBS} ${AUDIO_PLUGINS} ${VIDEO_PLUGINS}
do
        cpyX86 $i
done

# special case libaom
cpyLocal libaom.so.3

# subtitles
cpyX86 libass.so.9
for i in libfreetype.so.6 libfribidi.so.0 libfontconfig.so.1 libgraphite2.so.3
do
        cpyX86Optional $i
done

# Qt path
echo "[Paths]" > qt.conf
echo "Prefix=../lib/qt6" >> qt.conf

cd ..
find . -type f -exec sed -i -e 's|/usr/lib/x86_64-linux-gnu|./././././././././././lib|g' {} \; ; cd ..
find . -type f -exec sed -i -e 's|/usr/lib|././/lib|g' {} \;
#
popd > /dev/null
cp "$APPIMAGE_SCRIPT_DIR"/AppRunBookworm install/AppRun
cp "$APPIMAGE_SCRIPT_DIR"/avidemux.png install

FT_PROBE_EXE_NAME="freetype_probe"
FT_PROBE_LOCATION="buildPluginsCommon/ADM_videoFilters6/ass"
if [ -e "${FT_PROBE_LOCATION}/${FT_PROBE_EXE_NAME}" ]; then
    cp "${FT_PROBE_LOCATION}/${FT_PROBE_EXE_NAME}" install
    chmod +x "install/${FT_PROBE_EXE_NAME}"
fi

# Create launcher file
echo "[Desktop Entry]" > install/avidemux.desktop
echo "Name=Avidemux" >> install/avidemux.desktop
echo "Exec=avidemux3_qt6" >> install/avidemux.desktop
echo "Icon=avidemux" >> install/avidemux.desktop
echo "Type=Application" >> install/avidemux.desktop
echo "Categories=AudioVideo;AudioVideoEditing;Video;" >> install/avidemux.desktop
echo "MimeType=video/mpeg;video/x-mpeg;video/mp4;video/x-m4v;video/quicktime;video/3gp;video/mkv;video/x-matroska;video/webm;video/flv;video/x-flv;video/dv;video/x-msvideo;video/x-ms-wmv;video/x-ms-asf;video/x-anim;" >> install/avidemux.desktop
echo "StartupWMClass=avidemux3_qt6" >> install/avidemux.desktop

if [ -f /tmp/myappimage.squashfs ]; then
    rm -f /tmp/myappimage.squashfs || exit 1
fi
mksquashfs install /tmp/myappimage.squashfs -root-owned -noappend || fail mksquashfs
cp "${RUNTIME}" "${APP_NAME}"
cat /tmp/myappimage.squashfs >> "${APP_NAME}" || fail "Appending squashfs to runtime"
chmod +x "${APP_NAME}"
echo "AppImage created as ${APP_NAME}"
