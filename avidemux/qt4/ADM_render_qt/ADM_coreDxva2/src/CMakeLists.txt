include(admAsNeeded)
SET(ADM_dxva2_SRCS
ADM_coreDxva2.cpp
ADM_coreD3D.cpp
)
#*************************************************
#*************************************************
SET(ADM_LIB ADM_coreDxva26)
ADM_ADD_SHARED_LIBRARY(ADM_coreDxva26 ${ADM_dxva2_SRCS})
SET_TARGET_PROPERTIES(${ADM_LIB} PROPERTIES CXX_VISIBILITY_PRESET hidden)
SET_TARGET_PROPERTIES(${ADM_LIB} PROPERTIES VISIBLITY_INLINES_HIDDEN True)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 ADM_coreUI6 dxva2 ADM_coreImage6 ADM_editor6_interface)
ADM_INSTALL_LIB(${ADM_LIB})
