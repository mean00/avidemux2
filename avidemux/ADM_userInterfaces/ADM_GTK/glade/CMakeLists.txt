# Install glade file into FOO/lib/ADM_glade/
set(ADM_glade
gtk2_build.glade
)
INSTALL(FILES ${ADM_glade} DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/ADM_glade/")

