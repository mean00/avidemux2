include(admOpenGl)
SET(ADM_LIB ADM_UI${QT_LIBRARY_EXTENSION}6)

ADM_QT_WRAP_CPP(${ADM_LIB}_source
	T_bitrate.h  T_button.h  T_dialogFactory.h  T_filesel.h  T_menu.h  T_slider.h
	T_threadCount.h  T_toggle.h  T_notch.h)

SET(${ADM_LIB}_T
	T_bitrate.cpp  T_button.cpp  T_dialogFactory.cpp  T_filesel.cpp  T_menu.cpp
	T_slider.cpp  T_threadCount.cpp  T_toggle.cpp  T_notch.cpp
        T_flyDialogQt4.cpp 
        T_QCanvas.cpp
)
if(USE_OPENGL)
        INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}//ADM_openGL/include/)
        SET(${ADM_LIB}_T ${${ADM_LIB}_T} T_openGL.cpp)
endif(USE_OPENGL)

SET(${ADM_LIB}_SRCS
	FAC_bar.cpp    FAC_frame.cpp  FAC_integer.cpp  FAC_aspectRatio.cpp
	FAC_float.cpp  FAC_hex.cpp    FAC_readOnlyText.cpp  toolkit.cpp FAC_timeStamp.cpp
	FAC_matrix.cpp  DIA_none.cpp  ${${ADM_LIB}_source}  ${${ADM_LIB}_T})

add_compiler_export_flags()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
ADD_LIBRARY(${ADM_LIB} SHARED ${${ADM_LIB}_SRCS})
ADM_OPEN_GL(${ADM_LIB})
TARGET_LINK_LIBRARIES(${ADM_LIB} ADM_core6 ADM_coreUI6 ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ADM_render6_${QT_LIBRARY_EXTENSION} ADM_coreVideoFilter6 ${GL_LIBS})
ADM_INSTALL_LIB( ${ADM_LIB} )
