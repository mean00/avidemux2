SET(ADM_toolkit_SRCS
  ADM_audioQueue.cpp  ADM_packetQueue.cpp  automation.cpp  filesel.cpp)

ADD_LIBRARY(ADM_toolkit6 STATIC ${ADM_toolkit_SRCS})
TARGET_LINK_LIBRARIES(ADM_toolkit6 PRIVATE ADM_editor6_interface ADM_videoEncoder6_interface ADM_commonUI6_interface)
