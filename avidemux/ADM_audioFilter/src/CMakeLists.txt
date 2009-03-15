SET(ADM_audioFilter_SRCS
audiofilter_bridge.cpp
audiofilter_access.cpp
audiofilter_mixer.cpp
audiofilter_dolby.cpp
audiofilter_film2pal.cpp
audiofilter_limiter.cpp
audiofilter_normalize.cpp
audiofilter_SRC.cpp
audiofilter.cpp
)
ADD_LIBRARY(ADM_audioFilter STATIC ${ADM_audioFilter_SRCS})

