INCLUDE(vf_plugin)
INCLUDE(vf_plugin_qt4gl)


SET(LibGliphy_SRCS 
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-arc.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-arcs.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-blob.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-extents.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-outline.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-sdf.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-shaders.cc
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-arc-bezier.hh
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-arcs-bezier.hh
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-common.hh
${CMAKE_CURRENT_SOURCE_DIR}/glyphy/glyphy-geometry.hh
)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/glyphy)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)

SET(ADM_vf_GlGlyphy_SRCS glGlyphy.cpp ${LibGliphy_SRCS})

ADD_DEFINITIONS(-DPKGDATADIR="")

INIT_VIDEO_FILTER_GLQT4(ADM_vf_GlGlyphy "${ADM_vf_GlGlyphy_SRCS}" "" "glGlyphy" )

