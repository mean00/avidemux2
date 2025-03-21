MACRO(checkGtk)
	IF(NOT GTK_CHECKED)
		OPTION(GTK "" ON)

		MESSAGE(STATUS "Checking for GTK+")
		MESSAGE(STATUS "*****************")

		IF(GTK)
			PKG_CHECK_MODULES(PIXBUF gdk-pixbuf-2.0)
			PRINT_LIBRARY_INFO("GdkPixBuf" PIXBUF_FOUND "${PIXBUF_CFLAGS}" "${PIXBUF_LDFLAGS}")

			IF(NOT PIXBUF_FOUND)
				MESSAGE(STATUS "gdk-pixbuf not found")
				SET(GTK_FOUND false)
			ENDIF()

			PKG_CHECK_MODULES(GTK gtk+-3.0)
			PRINT_LIBRARY_INFO("GTK+" GTK_FOUND "${GTK_CFLAGS}" "${GTK_LDFLAGS}")
			MESSAGE("")

			IF(GTK_FOUND)
				ADM_COMPILE(gtk_x11_check.cpp "${GTK_CFLAGS}" "" "${GTK_LDFLAGS}" GTK_X11_SUPPORTED outputGtkX11Test)

				IF(GTK_X11_SUPPORTED)
					SET(HAVE_GTK_X11 1)

					MESSAGE(STATUS "GTK+ uses X11")
				ELSE()
					MESSAGE(STATUS "GTK+ doesn't use X11")

					IF(VERBOSE)
						MESSAGE("Error Message: ${outputGtkX11Test}")
					ENDIF()
				ENDIF()

				SET(GTK_CFLAGS ${GTK_CFLAGS} ${PIXBUF_CFLAGS})
 				SET(GTK_LDFLAGS ${GTK_LDFLAGS} ${PIXBUF_LDFLAGS})
				MESSAGE("")
			ENDIF()
		ELSE()
			MESSAGE("${MSG_DISABLE_OPTION}")
			MESSAGE("")
		ENDIF()

		MESSAGE(STATUS "Checking for GThread")
		MESSAGE(STATUS "********************")

		IF(GTK)
			PKG_CHECK_MODULES(GTHREAD gthread-2.0)
			PRINT_LIBRARY_INFO("GThread" GTHREAD_FOUND "${GTHREAD_CFLAGS}" "${GTHREAD_LDFLAGS}")

			IF(NOT GTHREAD_FOUND)
				MESSAGE(STATUS "Could not find GThread")
			ENDIF()
		ELSE()
			MESSAGE("${MSG_DISABLE_OPTION}")
		ENDIF()

		SET(GTK_CHECKED 1)

		MESSAGE("")
	ENDIF()
ENDMACRO()