ADD_SUBDIRECTORY(muxerDummy)
ADD_SUBDIRECTORY(muxerAvi)
ADD_SUBDIRECTORY(muxerRaw)
ADD_SUBDIRECTORY(muxerFlv)
ADD_SUBDIRECTORY(muxerMp4)
ADD_SUBDIRECTORY(muxerMkv)
ADD_SUBDIRECTORY(muxerffPS)
ADD_SUBDIRECTORY(muxerffTS)
#ADD_SUBDIRECTORY(muxerMplex)
IF(WIN32)
        IF(MSVC) # nope
        ELSE(MSVC) 
         IF ("${CROSS_C_COMPILER}" STREQUAL "clang") # Nope either
         ELSE ("${CROSS_C_COMPILER}" STREQUAL "clang")
                ADD_SUBDIRECTORY(muxerMp4v2)
         ENDIF ("${CROSS_C_COMPILER}" STREQUAL "clang")
        ENDIF(MSVC) 
        # With clang no exception in the compiler for the moment, so no mp4v2
ELSE(WIN32 )
        ADD_SUBDIRECTORY(muxerMp4v2)
ENDIF(WIN32)

