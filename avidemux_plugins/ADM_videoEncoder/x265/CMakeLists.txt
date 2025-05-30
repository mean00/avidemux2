
INCLUDE(ve_plugin)
INCLUDE(ve_settings_plugin)



SET(x265_SRCS
        ADM_x265.cpp
        ADM_x265Plugin.cpp
        ADM_x265Setup.cpp
        x265_settings_json.cpp
)
include(admCheckX265)
checkX265()
IF(USE_X265)

  MESSAGE(STATUS "x265 plugin activated, with cflags=${X265_CFLAGS}.")
  ADD_DEFINITIONS(${X265_CFLAGS})
  ADD_DEFINITIONS(-DSIMPLE_MODE_EX=1)
  INCLUDE_DIRECTORIES(${X265_INCLUDE_DIR})

  #-----------------------QT4 VERSION---------------------
  INCLUDE(ve_plugin_qt4)
  if(DO_QT4)
    SET(ADM_ve_x265_QT4_SRCS    qt4/Q_x265.cpp)
    SET(ADM_ve_x265_QT4_Headers qt4/Q_x265.h)
    SET(ADM_ve_x265_QT4_UI      qt4/x265ConfigDialog)
    ADD_VIDEO_ENCODER_QT4(ADM_ve_x265_${QT_LIBRARY_EXTENSION}   ${ADM_ve_x265_QT4_SRCS} ${ADM_ve_x265_QT4_Headers} ${ADM_ve_x265_QT4_UI} ${x265_SRCS})
    TARGET_LINK_LIBRARIES(ADM_ve_x265_${QT_LIBRARY_EXTENSION} PRIVATE "${X265_LIBRARY_DIR}")
    TARGET_LINK_LIBRARIES(ADM_ve_x265_${QT_LIBRARY_EXTENSION} PRIVATE ADM_core6)
    INIT_VIDEO_ENCODER(ADM_ve_x265_${QT_LIBRARY_EXTENSION})
    INSTALL_VIDEO_ENCODER_QT4(ADM_ve_x265_${QT_LIBRARY_EXTENSION})
  endif(DO_QT4)
  #-----------------------/QT4 VERSION---------------------
  #-----------------------GTK or CLI VERSION---------------------
  if(DO_COMMON)
    ADD_VIDEO_ENCODER(ADM_ve_x265_other   ADM_x265_noui.cpp ${x265_SRCS})
    ADD_TARGET_CFLAGS(ADM_ve_x265_other "-DADM_UI_TYPE_BUILD=1") # CLI!
    TARGET_LINK_LIBRARIES(ADM_ve_x265_other PRIVATE "${X265_LIBRARY_DIR}")
    TARGET_LINK_LIBRARIES(ADM_ve_x265_other PRIVATE ADM_core6)
    INIT_VIDEO_ENCODER(ADM_ve_x265_other)
    INSTALL_VIDEO_ENCODER(ADM_ve_x265_other)
  endif(DO_COMMON)
  # ------------- PRESET -----------
  if(DO_SETTINGS)
    FILE(GLOB json_files ${CMAKE_CURRENT_SOURCE_DIR}/preset/*.json)
    if(json_files)
      message(STATUS "Presets found : ${json_files}")
      INSTALL_VIDEO_ENCODER_SETTINGS(x265 VERSION 3 FILES ${json_files})
    endif(json_files)
  endif(DO_SETTINGS)
ELSE(USE_X265)
  MESSAGE(STATUS "x265 disabled.")
ENDIF(USE_X265)

MESSAGE("")
