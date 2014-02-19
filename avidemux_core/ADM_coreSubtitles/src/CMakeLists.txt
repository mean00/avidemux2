SET(ADM_core_subtitles_SRCS ADM_coreSubtitles.cpp )
INCLUDE_DIRECTORIES(../inc)

# We force the use of fontconfig
ADM_ADD_SHARED_LIBRARY(ADM_coreSubtitle ${ADM_core_subtitles_SRCS})
