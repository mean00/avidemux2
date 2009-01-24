PROJECT(mkv)
INCLUDE_DIRECTORIES( .) 
INCLUDE_DIRECTORIES( ../../avidemux/ADM_inputs/ADM_matroska)
#LINK_DIRECTORIES(${ITK_LIBRARY_DIRS})
ADD_DEFINITIONS("-g")


SET(sources
../../avidemux/ADM_inputs/ADM_matroska/ebml.cpp 
mkv_c.cpp 
../../avidemux/ADM_inputs/ADM_matroska/mkv_tags.cpp
)


ADD_EXECUTABLE(mkv ${sources})


