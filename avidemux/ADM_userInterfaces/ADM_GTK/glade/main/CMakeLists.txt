# Install glade file into FOO/lib/ADM_glade/
set(ADM_glade
gtk2_build.glade
about.xpm
avidemux_icon.xpm
backward.xpm
begin.xpm
end.xpm
forward.xpm
gnome-calculator.png
gnome-calculator_small.xpm
gnome-calculator.xpm
Kbackward.xpm
Kforward.xpm
markA.xpm
markB.xpm
play.xpm
stop.xpm
systray2.xpm
systray.xpm
xpm_nextblack.xpm
xpm_prevblack.xpm
avidemux_icon_small.png
audio-volume-medium.png
)
INSTALL(FILES ${ADM_glade} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ADM_glade/main")


