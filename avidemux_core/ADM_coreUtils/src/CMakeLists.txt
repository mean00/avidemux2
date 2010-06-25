SET(ADM_coreUtils_SRCS 
ADM_confCouple.cpp  
ADM_bitstream.cpp 
avidemutils.cpp  
ADM_quota.cpp
fourcc.cpp
ADM_infoExtractor.cpp
ADM_infoExtractorH264.cpp
ADM_frameType.cpp
ADM_codecType.cpp
ADM_file.cpp
ADM_paramList.cpp
ADM_coreCodecMapping.cpp
ADM_threadQueue.cpp
ADM_string.cpp
prefs.cpp)

ADD_LIBRARY(ADM_coreUtils6 SHARED ${ADM_coreUtils_SRCS})

TARGET_LINK_LIBRARIES(ADM_coreUtils6 ADM_core6 ADM_coreUI6 ADM_libavutil6 ADM_libavcodec6 ${LIBXML2_LIBRARIES})


ADD_SOURCE_CFLAGS(prefs.cpp -I${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS})
ADD_SOURCE_CFLAGS(ADM_quota.cpp -I${LIBXML2_INCLUDE_DIR} ${LIBXML2_DEFINITIONS})
ADM_INSTALL_LIB(ADM_coreUtils6)
