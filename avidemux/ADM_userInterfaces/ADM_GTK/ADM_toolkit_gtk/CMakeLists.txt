SET(ADM_LIB ADM_toolkitGtk)

SET(${ADM_LIB}_SRCS
	ADM_tray_gtk.cpp  
    gtkmarkscale.c jogshuttle.c ADM_jogshuttle.cpp  mediactrl.c
ADM_icons.cpp

)

ADD_ADM_LIB_GTK_TARGET(${ADM_LIB} ${${ADM_LIB}_SRCS})
ADD_CFLAGS_GTK_TARGET(${ADM_LIB} ${GTK_CFLAGS})
