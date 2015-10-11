include(admCheckVapourSynth)
include(dm_plugin)
checkVapourSynth()
IF(USE_VAPOURSYNTH)
    ADD_DEFINITIONS("-std=c++11")
    ADD_DEFINITIONS(${VAPOURSYNTHSCRIPT_CFLAGS})
    SET(ADM_vs_SRCS 
	ADM_vs.cpp
	ADM_vsPlugin.cpp
        )

    ADD_DEMUXER(ADM_dm_vs ${ADM_vs_SRCS})
    GET_TARGET_PROPERTY(OLD PROPERTIES LINK_FLAGS)
    IF("${OLD}" MATCHES ".*NOT.*FOUND")
        SET(OLD "")
    ENDIF("${OLD}" MATCHES ".*NOT.*FOUND")
# Keep -L, discard everything else
    FOREACH(i ${VAPOURSYNTHSCRIPT_LDFLAGS})
       MESSAGE(STATUS "Updating linker flags <${OLD}> => ${i}")
       IF(${i} MATCHES "^\-l")
                LIST(APPEND LIBS_TO_LINK  ${i})
       ENDIF(${i} MATCHES "^\-l")
       IF(${i} MATCHES "^\-L")
                SET(OLD "${i} ${OLD} ") 
       ENDIF(${i} MATCHES "^\-l")
    ENDFOREACH(i ${VAPOURSYNTHSCRIPT_LDFLAGS})
    SET_TARGET_PROPERTIES(ADM_dm_vs PROPERTIES LINK_FLAGS "${OLD}")

    INIT_DEMUXER(ADM_dm_vs)
    INSTALL_DEMUXER(ADM_dm_vs)
    TARGET_LINK_LIBRARIES(ADM_dm_vs ${LIBS_TO_LINK})
ENDIF(USE_VAPOURSYNTH)
