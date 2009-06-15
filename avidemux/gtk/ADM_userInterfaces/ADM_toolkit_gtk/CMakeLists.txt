SET(ADM_LIB ADM_toolkitGtk)

SET(${ADM_LIB}_SRCS
	ADM_tray_gtk.cpp  
    gtkmarkscale.c jogshuttle.c ADM_jogshuttle.cpp  mediactrl.c
ADM_icons.cpp

)

ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
