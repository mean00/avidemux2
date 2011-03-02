ADD_SUBDIRECTORY(main)
# Install glade file into FOO/lib/ADM_glade/
set(ADM_glade
encoding.gtkBuilder
)
INSTALL(FILES ${ADM_glade} DESTINATION "${AVIDEMUX_LIB_DIR}/ADM_glade/")

