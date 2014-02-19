SET(ADM_core_subtitles_SRCS ADM_coreSubtitles.cpp )
INCLUDE_DIRECTORIES(../inc)

TARGET_LINK_LIBRARIES(ADM_core_subtitles ADM_libass ${FREETYPE2_LDFLAGS} ${FRIDIBI_LDFLAGS})
# We force the use of fontconfig
ADM_ADD_SHARED_LIBRARY(ADM_coreSubtitle ${ADM_core_subtitles_SRCS})
