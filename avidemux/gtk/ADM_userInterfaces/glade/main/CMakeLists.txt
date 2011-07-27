# Install glade file into FOO/lib/ADM_glade/
set(ADM_glade
gtk2_build.gtkBuilder
avidemux_icon_small.png
)

FILE(GLOB ADM_icons ${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/common/ADM_icons/*.png)
INSTALL(FILES ${ADM_glade} ${ADM_icons} DESTINATION "${AVIDEMUX_LIB_DIR}/ADM_glade/main")
