# # # # # # # # # # # # # # # # # # # # # # # # # #################
# Config
# # # # # # # # # # # # # # # # # # # # # # # # # #################
SET(ADM_vsPROXY_SRCS
  ../ADM_vsProxy.cpp
  ../ADM_vsProxy.h
  vsProxy_qt4.h
  vsProxy_qt4.cpp
)
SET(EXE vsProxy_gui_${QT_EXTENSION})
ADD_EXECUTABLE(${EXE} ${ADM_vsPROXY_SRCS})
TARGET_COMPILE_OPTIONS(${EXE} PRIVATE ${VAPOURSYNTHSCRIPT_CFLAGS})
TARGET_COMPILE_DEFINITIONS(${EXE} PRIVATE "VAPOURSYNTH_PYTHONLIB=${VAPOURSYNTH_PYTHONLIB}")
#
IF(MSVC)
  TARGET_COMPILE_DEFINITIONS(${EXE} PRIVATE "NO_WINSOCK2") # else double include on windows
ENDIF()
#
TARGET_COMPILE_DEFINITIONS(${EXE} PRIVATE ADM_UI_TYPE_BUILD=ADM_UI_QT4)
SET(CONFIG_HEADER_TYPE ADM_BUILD_QT4)
SET(UI_SUFFIX qt4)
TARGET_INCLUDE_DIRECTORIES(${EXE} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
INCLUDE(admWindRes)
# # # # # # # # # # # # # # # # # # # # # # # # # ##################
# Executable
# # # # # # # # # # # # # # # # # # # # # # # # # ##################
#
SET(UI vs.ui)
SET(srcs vsProxy_qt4.h)
#ADM_QT_ADD_RESOURCES(resource  jobs.rsc)
ADM_QT_WRAP_UI(VS_PROXY_headers ${UI})
ADM_QT_WRAP_CPP(VS_PROXY_source ${srcs}  vsProxy_qt4.h)
TARGET_SOURCES(${EXE} PRIVATE ${VS_PROXY_headers} ${VS_PROXY_source})

# # # # # # # # # # # # # # # # # # # # # # # # # ##################
# Executable
# # # # # # # # # # # # # # # # # # # # # # # # # ##################
#
GET_PROPERTY(OLD
                        TARGET ${EXE}
                        PROPERTY LINK_FLAGS)
IF("${OLD}" MATCHES ".*NOT.*FOUND")
  SET(OLD "")
ENDIF()
# Keep -L, discard everything else

SET_TARGET_PROPERTIES(${EXE}  PROPERTIES LINK_FLAGS "${OLD}")

# # # # # # # # # # # # # # # # # # # # # # # # # ##################
TARGET_LINK_LIBRARIES(${EXE} PRIVATE  ADM_core6 ADM_coreSocket6 ADM_coreUtils6)
TARGET_LINK_LIBRARIES(${EXE} PRIVATE  ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY})
TARGET_LINK_LIBRARIES(${EXE} PRIVATE  ${LIBS_TO_LINK})

INCLUDE(admCheckThreads)
ADM_LINK_THREAD(${EXE})

IF(MINGW)
  TARGET_LINK_LIBRARIES(${EXE} PRIVATE -lwsock32 winmm -mwindows)
  TARGET_LINK_LIBRARIES(${EXE} PRIVATE ${LIBS_TO_LINK} "-lm -lstdc++")
ENDIF()
IF(MSVC)
  SET_TARGET_PROPERTIES(${EXE} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
  SET_TARGET_PROPERTIES(${EXE} PROPERTIES WIN32_EXECUTABLE True)
  TARGET_LINK_LIBRARIES(${EXE} PRIVATE ws2_32.lib  winmm -mwindows)
ENDIF()
TARGET_LINK_LIBRARIES(${EXE} PRIVATE "${VS_SYSTEM_LIBS}")
ADM_INSTALL_BIN(${EXE})
