SET(ADMaudiofilter_SRCS 
	audiodeng_buildfilters.cpp    
)
	
ADD_LIBRARY(ADM_audioFilter_old STATIC ${ADMaudiofilter_SRCS})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/avidemux/ADM_audioEncoder/include)

