SET(ADMaudiofilter_SRCS 
	audiodeng_buildfilters.cpp    
)
	
ADD_ADM_LIB_ALL_TARGETS(ADM_audioFilter_old ${ADMaudiofilter_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_audioEncoder/include)

