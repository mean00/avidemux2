#
# API
SET(ADM_audioFilterAPI_SRCS
audiofilterAPI.cpp
)
# For the API part, needed by coreAudioEncoder
ADD_LIBRARY(ADM_coreAudioFilterAPI6 SHARED ${ADM_audioFilterAPI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioFilterAPI6 ADM_core6 ADM_coreUI6  )
ADM_INSTALL_LIB(ADM_coreAudioFilterAPI6)
# END
