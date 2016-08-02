#
#  Macro to declare an video encoder plugin, qt4 flavor
#
include(plugin_qt4)
include(admAsNeeded)
MACRO(ADD_VIDEO_ENCODER_QT4 name srcQ headerQ uiQ)
        #INIT_VIDEO_ENCODER(${name})
        INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDE_DIR})
        ADM_ADD_QT_INCLUDE_DIR(${QT_EXTENSION} ADM_UIs)
        ADM_QT_WRAP_UI(qt4_ui ${uiQ}.ui)
        ADM_QT_WRAP_CPP(qt4_cpp ${headerQ})
        ADD_LIBRARY(${name} SHARED ${ARGN} ${srcQ} ${qt4_cpp} ${qt4_ui})
	AS_NEEDED(${name})
        ADD_TARGET_CFLAGS(${name} "-DADM_UI_TYPE_BUILD=4")
        TARGET_LINK_LIBRARIES( ${name} ADM_UI${QT_LIBRARY_EXTENSION}6 )
        TARGET_LINK_LIBRARIES(${name} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

ENDMACRO(ADD_VIDEO_ENCODER_QT4 )



MACRO(INSTALL_VIDEO_ENCODER_QT4 _lib)
	INSTALL(TARGETS ${_lib} 
                DESTINATION "${VE_PLUGIN_DIR}/${QT_EXTENSION}"
                COMPONENT  plugins
                )
	TARGET_LINK_LIBRARIES(${_lib} ADM_core6 ADM_coreUI6 ADM_coreVideoEncoder6 ADM_coreImage6 ADM_coreUtils6 m)
ENDMACRO(INSTALL_VIDEO_ENCODER_QT4)




