##########################################
# Config
##########################################
ADD_DEFINITIONS("-std=c++11")

ADD_DEFINITIONS(${VAPOURSYNTHSCRIPT_CFLAGS})
#
IF(MSVC)
		ADD_DEFINITIONS("-DNO_WINSOCK2") # else double include on windows	
ENDIF(MSVC)
#
ADD_DEFINITIONS(-DADM_UI_TYPE_BUILD=ADM_UI_QT4)
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX qt4)
include_directories(./ ${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE(admWindRes)
###########################################
# Executable
###########################################
#
SET(UI vs.ui )
SET(srcs vsProxy_qt4.h)
#ADM_QT_ADD_RESOURCES(resource  jobs.rsc)
ADM_QT_WRAP_UI(VS_PROXY_headers ${UI})
ADM_QT_WRAP_CPP(VS_PROXY_source ${srcs}  vsProxy_qt4.h )
SET(ADM_vsPROXY_SRCS 
                ${srcs}
                ../ADM_vsProxy.cpp
                ../ADM_vsProxy.h
               	#${resource} 
                ${VS_PROXY_headers}
                ${VS_PROXY_source}
                vsProxy_qt4.cpp
        )


###########################################
# Executable
###########################################
ADD_EXECUTABLE(vsProxy_gui_${QT_EXTENSION} ${ADM_vsPROXY_SRCS})
#
GET_PROPERTY(OLD 
                        TARGET vsProxy_gui_${QT_EXTENSION}
                        PROPERTY LINK_FLAGS)
IF("${OLD}" MATCHES ".*NOT.*FOUND")
   SET(OLD "")
ENDIF("${OLD}" MATCHES ".*NOT.*FOUND")
# Keep -L, discard everything else


SET_TARGET_PROPERTIES(vsProxy_gui_${QT_EXTENSION}  PROPERTIES LINK_FLAGS "${OLD}")

###########################################
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ADM_core6 ADM_coreSocket6 ADM_coreUtils6)
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ${PTHREAD_LIBRARIES})
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ${LIBS_TO_LINK} )

IF (MINGW)
	target_link_libraries(vsProxy_gui_${QT_EXTENSION} -lwsock32 winmm -mwindows )
	target_link_libraries(vsProxy_gui_${QT_EXTENSION} ${LIBS_TO_LINK} "-lm -lstdc++")
ENDIF (MINGW)
IF(MSVC)
		set_target_properties(vsProxy_gui_${QT_EXTENSION} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
		set_target_properties(vsProxy_gui_${QT_EXTENSION} PROPERTIES WIN32_EXECUTABLE True)
		TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ws2_32.lib  winmm -mwindows  )
ENDIF(MSVC)
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} "${VS_SYSTEM_LIBS}")
ADM_INSTALL_BIN(vsProxy_gui_${QT_EXTENSION})
