INCLUDE(admCheckVapourSynth)
INCLUDE(dm_plugin)
checkVapourSynth()

IF(NOT APPLE AND NOT WIN32 AND NOT MSVC)
  SET(VS_SYSTEM_LIBS "-ldl")
ENDIF()


IF(USE_VAPOURSYNTH)
  IF(DO_COMMON)
    IF(NOT APPLE)
      IF(MSVC)
        ADD_DEFINITIONS("-DNO_WINSOCK2") # else double include on windows
      ENDIF()
      #ADD_DEFINITIONS("-g3 -O0")
      #ADD_DEFINITIONS("-std=c++11")
      SET(SRCS ADM_vsProxy.cpp ADM_vsProxy_cli.cpp)
      ADD_EXECUTABLE(vsProxy ${SRCS})
      # Add vs Proxy
      SET_TARGET_PROPERTIES(vsProxy PROPERTIES LINK_FLAGS "${OLD}")
      TARGET_COMPILE_OPTIONS(vsProxy PRIVATE ${VAPOURSYNTHSCRIPT_CFLAGS})
      TARGET_COMPILE_DEFINITIONS(vsProxy PRIVATE "VAPOURSYNTH_PYTHONLIB=${VAPOURSYNTH_PYTHONLIB}")
      TARGET_LINK_LIBRARIES(vsProxy PRIVATE ADM_core6 ADM_coreSocket6)
    ENDIF()
    #----------------
    # Add direct Loading
    SET(ADM_vs_SRCS
                ADM_vs.cpp  ADM_vs.h  ADM_vsPlugin.cpp
                )
    INIT_DEMUXER(ADM_dm_vapoursynth)
    ADD_DEMUXER(ADM_dm_vapoursynth ${ADM_vs_SRCS})
    TARGET_COMPILE_OPTIONS(ADM_dm_vapoursynth PRIVATE ${VAPOURSYNTHSCRIPT_CFLAGS})
    TARGET_COMPILE_DEFINITIONS(ADM_dm_vapoursynth PRIVATE "VAPOURSYNTH_PYTHONLIB=${VAPOURSYNTH_PYTHONLIB}")
    TARGET_LINK_LIBRARIES(ADM_dm_vapoursynth PRIVATE "${VS_SYSTEM_LIBS}")

    INSTALL_DEMUXER(ADM_dm_vapoursynth)
    #----------------
    IF(NOT APPLE)
      IF(WIN32)
        IF(MSVC)
          TARGET_LINK_LIBRARIES(vsProxy PRIVATE ws2_32.lib)
        ELSE()
          TARGET_LINK_LIBRARIES(vsProxy PRIVATE wsock32 winmm -mwindows m stdc++)
        ENDIF()
      ELSE()
        TARGET_LINK_LIBRARIES(vsProxy PRIVATE -lstdc++) # clang
      ENDIF()
      TARGET_LINK_LIBRARIES(vsProxy PRIVATE "${VS_SYSTEM_LIBS}")
      ADM_INSTALL_BIN(vsProxy)
    ENDIF()
  ENDIF()
  IF(DO_QT4 AND NOT APPLE)
    ADD_SUBDIRECTORY(qt4)
  ENDIF()
ENDIF()
