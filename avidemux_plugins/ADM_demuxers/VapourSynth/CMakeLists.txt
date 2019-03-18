include(admCheckVapourSynth)
include(dm_plugin)
checkVapourSynth()

 IF(NOT APPLE AND NOT WIN32 AND NOT MSVC)
           SET(VS_SYSTEM_LIBS "-ldl")
ENDIF(NOT APPLE AND NOT WIN32 AND NOT MSVC)


IF(USE_VAPOURSYNTH)
    IF(DO_COMMON)
        IF(NOT APPLE)
            IF(MSVC)
                ADD_DEFINITIONS("-DNO_WINSOCK2") # else double include on windows
            ENDIF(MSVC)
            #ADD_DEFINITIONS("-g3 -O0")
            ADD_DEFINITIONS("-std=c++11")
            #ADD_DEFINITIONS("-std=c++11")
            ADD_DEFINITIONS(${VAPOURSYNTHSCRIPT_CFLAGS})
            SET(SRCS ADM_vsProxy.cpp ADM_vsProxy_cli.cpp)
            ADD_EXECUTABLE(vsProxy ${SRCS})                      
            # Add vs Proxy
            SET_TARGET_PROPERTIES(vsProxy PROPERTIES LINK_FLAGS "${OLD}")
            TARGET_LINK_LIBRARIES(vsProxy ADM_core6 ADM_coreSocket6)
        ELSE(NOT APPLE)
            ADD_DEFINITIONS(${VAPOURSYNTHSCRIPT_CFLAGS})
        ENDIF(NOT APPLE)
        #----------------
        # Add direct Loading
        SET(ADM_vs_SRCS 
                ADM_vs.cpp  ADM_vs.h  ADM_vsPlugin.cpp  
                )
        INIT_DEMUXER(ADM_dm_vapoursynth)
        ADD_DEMUXER(ADM_dm_vapoursynth ${ADM_vs_SRCS})
        TARGET_LINK_LIBRARIES(ADM_dm_vapoursynth "${VS_SYSTEM_LIBS}")

        INSTALL_DEMUXER(ADM_dm_vapoursynth)
        #----------------
        IF(NOT APPLE)
            IF(WIN32)
                IF(MSVC)
                    TARGET_LINK_LIBRARIES(vsProxy ws2_32.lib)
                ELSE(MSVC)
                    TARGET_LINK_LIBRARIES(vsProxy -lwsock32 winmm -mwindows -lm -lstdc++)
                ENDIF(MSVC)
            ELSE(WIN32)
                TARGET_LINK_LIBRARIES(vsProxy -lstdc++) # clang
            ENDIF(WIN32)
            TARGET_LINK_LIBRARIES(vsProxy "${VS_SYSTEM_LIBS}")
            ADM_INSTALL_BIN(vsProxy)
        ENDIF(NOT APPLE)
   ENDIF(DO_COMMON)
   IF(DO_QT4 AND NOT APPLE)
        ADD_SUBDIRECTORY(qt4)
   ENDIF(DO_QT4 AND NOT APPLE)
ENDIF(USE_VAPOURSYNTH)
