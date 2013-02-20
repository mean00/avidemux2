
INCLUDE(ve_plugin)

SET(x264_SRCS 
        ADM_x264.cpp
        ADM_x264Plugin.cpp
        ADM_x264Setup.cpp
        x264_encoder_json.cpp
)
include(admCheckX264)
checkX264()
IF(X264_FOUND)
  MESSAGE(STATUS "x264 plugin activated.")
  INCLUDE_DIRECTORIES(${X264_INCLUDE_DIR})

  #-----------------------QT4 VERSION---------------------
  INCLUDE(ve_plugin_qt4)
  if(DO_QT4)
    SET(ADM_ve_x264_QT4_SRCS    qt4/Q_x264.cpp)
    SET(ADM_ve_x264_QT4_Headers qt4/Q_x264.h)
    SET(ADM_ve_x264_QT4_UI      qt4/x264ConfigDialog)
    ADD_VIDEO_ENCODER_QT4(ADM_ve_x264_qt4   ${ADM_ve_x264_QT4_SRCS} ${ADM_ve_x264_QT4_Headers} ${ADM_ve_x264_QT4_UI} ${x264_SRCS})
    TARGET_LINK_LIBRARIES(ADM_ve_x264_qt4 "${X264_LIBRARY_DIR}")
    TARGET_LINK_LIBRARIES(ADM_ve_x264_qt4 ADM_core6)
    INIT_VIDEO_ENCODER(ADM_ve_x264_qt4)
    INSTALL_VIDEO_ENCODER(ADM_ve_x264_qt4)
  endif(DO_QT4)
  #-----------------------/QT4 VERSION---------------------
  #-----------------------GTK or CLI VERSION---------------------
  if((DO_GTK) OR (DO_CLI))
    ADD_VIDEO_ENCODER(ADM_ve_x264_other   ADM_x264_noui.cpp ${x264_SRCS})
    ADD_TARGET_CFLAGS(ADM_ve_x264_other "-DADM_UI_TYPE_BUILD=1") # CLI!
    TARGET_LINK_LIBRARIES(ADM_ve_x264_other "${X264_LIBRARY_DIR}")
    TARGET_LINK_LIBRARIES(ADM_ve_x264_other ADM_core6)
    INIT_VIDEO_ENCODER(ADM_ve_x264_other)
    INSTALL_VIDEO_ENCODER(ADM_ve_x264_other)
  endif((DO_GTK) OR (DO_CLI))
  # ------------- PRESET -----------
  if((DO_GTK) OR (DO_CLI) OR (DO_QT4))
    SET(PRESET_DIR "${AVIDEMUX_LIB_DIR}/${ADM_PLUGIN_DIR}/pluginSettings/x264/1/")

    FILE(GLOB json_files ${CMAKE_CURRENT_SOURCE_DIR}/preset/*.json)
    message(STATUS "Presets found : ${json_files}")
    install(FILES ${json_files} DESTINATION "${PRESET_DIR}")
  endif((DO_GTK) OR (DO_CLI) OR (DO_QT4))
ELSE(X264_FOUND)
  MESSAGE(STATUS "x264 disabled.")
ENDIF(X264_FOUND)

