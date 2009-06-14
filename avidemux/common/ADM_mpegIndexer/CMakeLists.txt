SET(ADM_mpegIndexer_SRCS
        dmx_indexer.cpp
        dmx_indexer_h264.cpp
        dmx_indexer_mpeg2.cpp
)
ADD_ADM_LIB_ALL_TARGETS(ADM_mpegIndexer ${ADM_mpegIndexer_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../ADM_coreDemuxerMpeg/include)
