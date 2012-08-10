#
# API
SET(ADM_audioFilterAPI_SRCS
audiofilterAPI.cpp
)

# For the API part, needed by coreAudioEncoder
add_compiler_export_flags()
ADM_ADD_SHARED_LIBRARY(ADM_coreAudioFilterAPI6 ${ADM_audioFilterAPI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioFilterAPI6 ADM_core6 ADM_coreUI6  )
ADM_INSTALL_LIB(ADM_coreAudioFilterAPI6)
# END
