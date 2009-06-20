SET(ADM_LIB ADM_render_${UI_SUFFIX})

SET(${ADM_LIB}_SRCS
GUI_render.cpp
GUI_sdlRender.cpp
GUI_xvRender.cpp
)

IF (APPLE)
	SET(${ADM_LIB}_SRCS ${${ADM_LIB}_SRCS} GUI_sdlRenderHelper.m)
ENDIF (APPLE)

ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
SDLify(GUI_sdlRender.cpp)
ADM_INSTALL_LIB(${ADM_LIB})
