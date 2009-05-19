include(mx_plugin)
SET(ADM_raw_SRCS 
	muxerRaw.cpp
	muxerRawPlugin.cpp
)

ADD_LIBRARY(ADM_mx_raw SHARED ${ADM_raw_SRCS})

INIT_MUXER(ADM_mx_raw)
INSTALL_MUXER(ADM_mx_raw)

