#
#  Macro to declare an video encoder plugin, qt4 flavor
#
include(admAsNeeded)
MACRO(ADD_VIDEO_ENCODER_QT4 name srcQ headerQ uiQ)
        #INIT_VIDEO_ENCODER(${name})
        INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${QT_INCLUDE_DIR})
		INCLUDE_DIRECTORIES(${AVIDEMUX_TOP_SOURCE_DIR}/avidemux/qt4/ADM_UIs/include/)
        QT4_WRAP_UI(qt4_ui ${uiQ}.ui)
        QT4_WRAP_CPP(qt4_cpp ${headerQ})
        ADD_LIBRARY(${name} SHARED ${ARGN} ${srcQ} ${qt4_cpp} ${qt4_ui})
	AS_NEEDED(${name})
        ADD_TARGET_CFLAGS(${name} "-DADM_UI_TYPE_BUILD=4")
        TARGET_LINK_LIBRARIES( ${name} ADM_UIQT46 )
        TARGET_LINK_LIBRARIES(${name} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})

ENDMACRO(ADD_VIDEO_ENCODER_QT4 )



