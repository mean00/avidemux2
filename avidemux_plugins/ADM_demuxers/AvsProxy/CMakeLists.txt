include(dm_plugin)
SET(ADM_avsproxy_SRCS 
	ADM_avsproxy.cpp  
        ADM_avsproxy_net.cpp
        ADM_avsPlugin.cpp
        ADM_avsproxyAudio.cpp
)
ADD_DEMUXER(ADM_dm_avsproxy ${ADM_avsproxy_SRCS})
if(WIN32)
        target_link_libraries(ADM_dm_avsproxy wsock32)
endif(WIN32)
INIT_DEMUXER(ADM_dm_avsproxy)
INSTALL_DEMUXER(ADM_dm_avsproxy)

