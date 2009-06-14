include(mx_plugin)
SET(ADM_avi_SRCS 
        muxerAvi.cpp        
        muxerAviPlugin.cpp   
        muxerAviConfig.cpp
        op_aviwrite.cpp
        avilist.cpp

#	avilist.cpp  op_avisavecopy_pack.cpp   op_avisave.cpp   op_aviwrite.cpp   op_savesmart.cpp
#	op_avisavecopy.cpp  op_avisavecopy_unpack.cpp  op_avisavedual.cpp  op_saveprocess.cpp
)
INCLUDE_DIRECTORIES("${AVIDEMUX_SOURCE_DIR}/avidemux/ADM_coreDemuxer/include")

ADD_LIBRARY(ADM_mx_avi SHARED ${ADM_avi_SRCS})

INIT_MUXER(ADM_mx_avi)
INSTALL_MUXER(ADM_mx_avi)
