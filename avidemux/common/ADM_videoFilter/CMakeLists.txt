SET(ADM_videoFilter_SRCS 
#
#	ADM_vidDvbSub.cpp
	#ADM_vidVobsub.cpp
	#ADM_vidVobSubRender.cpp
	#ADM_vobsubinfo.cpp
	ADM_vidComputeAverage.cpp
	ADM_vidThreshold.cpp
	ADM_vidNumber.cpp)

ADD_LIBRARY(ADM_videoFilter STATIC ${ADM_videoFilter_SRCS})

