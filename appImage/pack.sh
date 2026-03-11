cat <<EOF >AppDir/usr/share/metainfo/org.avidemux.Avidemux.appdata.xml
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>org.avidemux.Avidemux</id>
  
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-2.0+</project_license>
  
  <name>Avidemux</name>
  <summary>Free video editor</summary>
  
  <description>
    <p>
      Avidemux is a free video editor designed for simple cutting, filtering and encoding tasks. 
      It supports many file types, including AVI, DVD compatible MPEG files, MP4 and ASF.
    </p>
  </description>

  <url type="homepage">https://www.avidemux.org/</url>

  <developer>
    <name>Avidemux Team</name>
  </developer>

  <content_rating type="oars-1.1" />

  <launchable type="desktop-id">org.avidemux.Avidemux.desktop</launchable>
</component>

EOF
export APPRUN_FORCE_UPDATE=1
# OR more effectively for appstream specifically:
export VALIDATE=0
#export LD_LIBRARY_PATH=$PWD/AppDir/usr/lib
#linuxdeploy --appdir=AppDir --deploy-deps-only=AppDir/usr/lib/ --output appimage
export LDAI_RUNTIME_FILE=/usr/local/bin/runtime-x86_64
export EXTRA_QT_PLUGINS="wayland-egl;wayland;xcb"
cp appImage/AppRun_qt6 AppDir/AppRun
cp appImage/avidemux.png AppDir/
cp appImage/avidemux.desktop AppDir/
chmod +x AppDir/AppRun

appimagetool --runtime-file $LDAI_RUNTIME_FILE AppDir
chmod +x Avidem*.AppImage
#linuxdeploy -i appImage/avidemux.png \
#-d appImage/avidemux.desktop \
#--appdir=AppDir \
#--deploy-deps-only=AppDir/usr/lib/

#-e AppDir/usr/bin/avidemux3_qt6

#--plugin qt --output appimage
#appimage-builder --recipe appImage/avidemux.yml
