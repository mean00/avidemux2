SET(ADM_LIB ADM_toolkitGtk)

SET(${ADM_LIB}_SRCS
	ADM_tray_gtk.cpp  
        gtkmarkscale.c jogshuttle.c ADM_jogshuttle.cpp  mediactrl.c
        ADM_icons.cpp
        GUI_glade.cpp

)

ADD_DEFINITIONS("-DTARGET_DIR='\"${CMAKE_INSTALL_PREFIX}/lib/ADM_glade/\"' -DSOURCE_DIR='\"${CMAKE_CURRENT_SOURCE_DIR}\"'")
ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
ADM_INSTALL_LIB(${ADM_LIB})
