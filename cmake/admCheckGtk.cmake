# we also need libglade from now on
# So we search for both, them merge the flags into the gtk one
include(admCheckGlade)

MACRO(checkGtk)
	IF (NOT GTK_CHECKED)
		OPTION(GTK "" ON)

		MESSAGE(STATUS "Checking for GTK+")
		MESSAGE(STATUS "*****************")

		IF (GTK)
			PKG_CHECK_MODULES(PIXBUF gdk-pixbuf-2.0)
			PRINT_LIBRARY_INFO("GdkPixBuf" PIXBUF_FOUND "${PIXBUF_CFLAGS}" "${PIXBUF_LDFLAGS}")
                        if  (PIXBUF_FOUND)
                        else(PIXBUF_FOUND)
                                MESSAGE(STATUS "gdk-pixbuf not found")
                                set(GTK_FOUND false)
                        endif(PIXBUF_FOUND)
        
			PKG_CHECK_MODULES(GTK gtk+-2.0)
			PRINT_LIBRARY_INFO("GTK+" GTK_FOUND "${GTK_CFLAGS}" "${GTK_LDFLAGS}")
                        checkGlade()
                        if  (GLADE_FOUND)
                        else(GLADE_FOUND)
                                set(GTK_FOUND false)
                        endif(GLADE_FOUND)
                        
			IF (GTK_FOUND)
				ADM_COMPILE(gtk_x11_check.cpp "${GTK_CFLAGS}" "" "${GTK_LDFLAGS}" GTK_X11_SUPPORTED outputGtkX11Test)

				IF (GTK_X11_SUPPORTED)
					SET(HAVE_GTK_X11 1)

					MESSAGE(STATUS "GTK+ uses X11")
				ELSE (GTK_X11_SUPPORTED)
					MESSAGE(STATUS "GTK+ doesn't use X11")

					IF (VERBOSE)
						MESSAGE("Error Message: ${outputGtkX11Test}")
					ENDIF (VERBOSE)
				ENDIF (GTK_X11_SUPPORTED)
                               # Merge glade flags into gtk flags
                                SET(  GTK_CFLAGS "${GTK_CFLAGS} ${GLADE_CFLAGS} ${PIXBUF_CFLAGS}")
                                SET(  GTK_LDFLAGS "${GTK_LDFLAGS} ${GLADE_LDFLAGS} ${PIXBUF_LDFLAGS}")
                               # /Merge glade flags into gtk flags
			ENDIF (GTK_FOUND)
		ELSE (GTK)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (GTK)

		MESSAGE("")

		MESSAGE(STATUS "Checking for GThread")
		MESSAGE(STATUS "********************")

		IF (GTK)
			PKG_CHECK_MODULES(GTHREAD gthread-2.0)
			PRINT_LIBRARY_INFO("GThread" GTHREAD_FOUND "${GTHREAD_CFLAGS}" "${GTHREAD_LDFLAGS}")

			IF (NOT GTHREAD_FOUND)
				MESSAGE(STATUS "Could not find GThread")
			ENDIF(NOT GTHREAD_FOUND)
		ELSE (GTK)
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF (GTK)

		SET(GTK_CHECKED 1)

		MESSAGE("")
	ENDIF (NOT GTK_CHECKED)
ENDMACRO(checkGtk)
