SET(ADM_core_subtitles_SRCS ADM_coreSubtitles.cpp ADM_subLoader.cpp ADM_subConvert.cpp)
# We force the use of fontconfig
ADM_ADD_SHARED_LIBRARY(ADM_coreSubtitles6 ${ADM_core_subtitles_SRCS})
TARGET_INCLUDE_DIRECTORIES(ADM_coreSubtitles6 PUBLIC ../include)
TARGET_LINK_LIBRARIES(ADM_coreSubtitles6 PUBLIC ADM_core6)
ADM_INSTALL_LIB(ADM_coreSubtitles6)
