#
# API
SET(ADM_audioFilterAPI_SRCS
audiofilterAPI.cpp
)
# For the API part, needed by coreAudioEncoder
ADD_LIBRARY(ADM_coreAudioFilterAPI SHARED ${ADM_audioFilterAPI_SRCS})
TARGET_LINK_LIBRARIES(ADM_coreAudioFilterAPI ADM_core ADM_coreUI  )
INSTALL(TARGETS ADM_coreAudioFilterAPI RUNTIME DESTINATION ${BIN_DIR}  LIBRARY DESTINATION lib  ARCHIVE DESTINATION lib)
# END
