include(dm_plugin)
SET(ADM_mp4_SRCS 
ADM_atom.cpp 
ADM_mp4Analyzer.cpp 
ADM_mp4Fragments.cpp 
ADM_mp4audio.cpp 
ADM_mp4.cpp 
ADM_mp4Indexer.cpp 
ADM_mp4Leaf.cpp
ADM_mp4Plugin.cpp
)
	
ADD_DEMUXER(ADM_dm_mp4 ${ADM_mp4_SRCS})
TARGET_LINK_LIBRARIES(ADM_dm_mp4 ADM_audioParser6)
INIT_DEMUXER(ADM_dm_mp4)
INSTALL_DEMUXER(ADM_dm_mp4)
