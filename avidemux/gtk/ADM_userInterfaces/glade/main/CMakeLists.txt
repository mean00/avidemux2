# Install glade file into FOO/lib/ADM_glade/
set(ADM_glade
gtk2_build.gtkBuilder
gtk2_build.glade
avidemux_icon_small.png
audio-volume-medium.png
)

FILE(GLOB ADM_icons ${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_icons/*.png)
INSTALL(FILES ${ADM_glade} ${ADM_icons} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ADM_glade/main")


