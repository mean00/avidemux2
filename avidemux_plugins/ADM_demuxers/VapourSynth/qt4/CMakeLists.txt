##########################################
# Config
##########################################
ADD_DEFINITIONS("-std=c++11")

ADD_DEFINITIONS(${VAPOURSYNTHSCRIPT_CFLAGS})
GET_TARGET_PROPERTY(OLD PROPERTIES LINK_FLAGS)
IF("${OLD}" MATCHES ".*NOT.*FOUND")
   SET(OLD "")
ENDIF("${OLD}" MATCHES ".*NOT.*FOUND")
# Keep -L, discard everything else
FOREACH(i ${VAPOURSYNTHSCRIPT_LDFLAGS})
  IF(${i} MATCHES "^-L")
        MESSAGE(STATUS "Updating linker flags  => ${i}")
        SET(OLD "${i} ${OLD} ") 
  ELSE(${i} MATCHES "^-L")
        MESSAGE(STATUS "Updating linker libs  => ${i}")
        LIST(APPEND LIBS_TO_LINK  ${i})
  ENDIF(${i} MATCHES "^-L")
ENDFOREACH(i ${VAPOURSYNTHSCRIPT_LDFLAGS})

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
SET_TARGET_PROPERTIES(vsProxy_gui_${QT_EXTENSION}  PROPERTIES LINK_FLAGS "${OLD}")

###########################################
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ADM_core6 ADM_coreSocket6)
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ${PTHREAD_LIBRARIES})
TARGET_LINK_LIBRARIES(vsProxy_gui_${QT_EXTENSION} ${LIBS_TO_LINK} )

IF (MINGW)
	target_link_libraries(vsProxy_gui_${QT_EXTENSION} -lwsock32 winmm -mwindows )
ENDIF (MINGW)
target_link_libraries(vsProxy_gui_${QT_EXTENSION} ${LIBS_TO_LINK} "-lm -lstdc++")
ADM_INSTALL_BIN(vsProxy_gui_${QT_EXTENSION})
