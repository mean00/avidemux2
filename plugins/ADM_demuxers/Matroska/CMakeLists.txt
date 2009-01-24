include(dm_plugin)
SET(ADM_matroska_SRCS 
    ADM_mkv_audio.cpp  
    ADM_mkv.cpp  
    ADM_mkvEntries.cpp  
    ADM_mkvIndexer.cpp  
    ADM_mkvTrackType.cpp  
    ADM_mkvPlugin.cpp  
    ebml.cpp  
    mkv_tags.cpp
)
ADD_LIBRARY(ADM_dm_matroska SHARED ${ADM_matroska_SRCS})

TARGET_LINK_LIBRARIES(ADM_dm_matroska ADM_libavutil)

INIT_DEMUXER(ADM_dm_matroska)
INSTALL_DEMUXER(ADM_dm_matroska)

