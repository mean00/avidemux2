#
# API
SET(ADM_audioFilterAPI_SRCS
audiofilterAPI.cpp
)
# For the API part, needed by coreAudioEncoder
ADD_LIBRARY(ADM_coreAudioFilterAPI SHARED ${ADM_audioFilterAPI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioFilterAPI ADM_core ADM_coreUI  )
ADM_INSTALL_LIB(ADM_coreAudioFilterAPI)
# END
