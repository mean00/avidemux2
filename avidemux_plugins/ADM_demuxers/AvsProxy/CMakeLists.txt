include(dm_plugin)
SET(ADM_avsproxy_SRCS 
	ADM_avsproxy.cpp  
        ADM_avsproxy_net.cpp
        ADM_avsPlugin.cpp
)
ADD_LIBRARY(ADM_dm_avsproxy SHARED ${ADM_avsproxy_SRCS})
INIT_DEMUXER(ADM_dm_avsproxy)
INSTALL_DEMUXER(ADM_dm_avsproxy)

