include(mx_plugin)
SET(ADM_avi_SRCS 
        muxerAvi.cpp        
        muxerAviPlugin.cpp   
        muxerAviConfig.cpp
        op_aviwrite.cpp
        avilist.cpp
        avilist_avi.cpp
	avi_utils.cpp

)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_coreDemuxer/include")

ADD_MUXER(ADM_mx_avi ${ADM_avi_SRCS})

INIT_MUXER(ADM_mx_avi)
INSTALL_MUXER(ADM_mx_avi)
