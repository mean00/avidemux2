SET(ADM_internalVideoFilter_SRCS
        ADM_pluginLoad.cpp
        ADM_videoFilters.cpp
        ADM_filterThread.cpp
        ADM_vidPartial.cpp
)
IF (USE_VDPAU)
  ADD_CORE_INCLUDE(ADM_coreVdpau)
ENDIF (USE_VDPAU)
IF (USE_LIBVA)
  ADD_CORE_INCLUDE(ADM_coreLibVA)
ENDIF (USE_LIBVA)

ADD_LIBRARY(ADM_videoFilter6_interface INTERFACE)
TARGET_INCLUDE_DIRECTORIES(ADM_videoFilter6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

ADD_LIBRARY(ADM_internalVideoFilter6 STATIC ${ADM_internalVideoFilter_SRCS})
TARGET_LINK_LIBRARIES(ADM_internalVideoFilter6 PRIVATE ADM_editor6_interface ADM_videoFilter6_interface)

