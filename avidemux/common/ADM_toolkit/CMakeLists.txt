SET(ADM_LIB ADM_toolkit6)
ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_audioQueue.cpp
  ADM_packetQueue.cpp
  automation.cpp
  filesel.cpp)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_editor6_interface ADM_videoEncoder6_interface ADM_commonUI6_interface adm_pthread)
#

