SET(ADM_audioFilter_SRCS
audiofilter_bridge.cpp
audiofilter_access.cpp
audiofilter_mixer.cpp
audiofilter.cpp
)
ADD_LIBRARY(ADM_audioFilter STATIC ${ADM_audioFilter_SRCS})

