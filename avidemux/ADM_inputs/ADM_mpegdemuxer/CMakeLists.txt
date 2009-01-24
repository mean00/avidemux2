SET(ADM_mpegdemuxer_SRCS 
 dmx_audio.cpp   
 dmx_demuxerES.cpp    
 dmx_demuxerPS.cpp 
 dmx_identify.cpp 
 dmx_indexer_h264.cpp  
 dmx_io.cpp    
 dmx_probeTS.cpp
 dmx_demuxer.cpp 
# dmx_demuxerMSDVR.cpp 
 dmx_demuxerTS.cpp 
 dmx_indexer.cpp  
 dmx_indexer_mpeg2.cpp 
 dmx_probe.cpp 
 dmx_video.cpp)

ADD_ADM_LIB_ALL_TARGETS(ADM_mpegdemuxer ${ADM_mpegdemuxer_SRCS})
