SET(ADM_core_subtitles_SRCS ADM_coreSubtitles.cpp ADM_subLoader.cpp)
INCLUDE_DIRECTORIES(../inc)

# We force the use of fontconfig
ADM_ADD_SHARED_LIBRARY(ADM_coreSubtitle ${ADM_core_subtitles_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreSubtitle ADM_core6 )
ADM_INSTALL_LIB(ADM_coreSubtitle)
