SET(ADM_coreUtils_SRCS 
ADM_bitstream.cpp 
avidemutils.cpp  
ADM_quota.cpp
fourcc.cpp
ADM_infoExtractor.cpp
ADM_frameType.cpp
ADM_codecType.cpp
prefs.cpp)

ADD_LIBRARY(ADM_coreUtils SHARED ${ADM_coreUtils_SRCS})

TARGET_LINK_LIBRARIES(ADM_coreUtils ADM_core ADM_coreUI ADM_libavutil ADM_libavcodec ${LIBXML2_LIBRARIES})


ADD_SOURCE_CFLAGS(prefs.cpp -I${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS})
ADD_SOURCE_CFLAGS(ADM_quota.cpp -I${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS})
ADM_INSTALL_LIB(ADM_coreUtils)
