# Demuxer for mxfture
# PNG etc...

include(dm_plugin)

SET(ADM_mxf_SRCS 
	ADM_mxfPlugin.cpp
	ADM_mxf.cpp
)

ADD_DEMUXER(ADM_dm_mxf ${ADM_mxf_SRCS})

INIT_DEMUXER(ADM_dm_mxf)
INSTALL_DEMUXER(ADM_dm_mxf)

