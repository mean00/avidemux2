SET(ADM_LIB ADM_internalVideoFilter6)
ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_pluginLoad.cpp
  ADM_videoFilters.cpp
  ADM_filterThread.cpp
  ADM_vidPartial.cpp
  ADM_coreVideoFilterFunc.cpp
  ADM_videoFilterBridge.cpp
)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_editor6_interface ADM_videoFilter6_interface adm_gettext)
#
IF (USE_VDPAU)
  ADD_CORE_INCLUDE(ADM_coreVdpau)
ENDIF (USE_VDPAU)
IF (USE_LIBVA)
  ADD_CORE_INCLUDE(ADM_coreLibVA)
ENDIF (USE_LIBVA)


ADD_LIBRARY(ADM_videoFilter6_interface INTERFACE)
TARGET_INCLUDE_DIRECTORIES(ADM_videoFilter6_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)

