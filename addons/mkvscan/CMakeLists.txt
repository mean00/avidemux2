PROJECT(mkv)
INCLUDE_DIRECTORIES( .) 
set(MKV ../../avidemux_plugins/ADM_demuxers/Matroska/)
INCLUDE_DIRECTORIES( ${MKV})
#LINK_DIRECTORIES(${ITK_LIBRARY_DIRS})
ADD_DEFINITIONS("-g -DADM_fopen=fopen")
ADD_DEFINITIONS("-D__STDC_FORMAT_MACROS")


SET(sources
${MKV}ebml.cpp 
mkv_c.cpp 
${MKV}mkv_tags.cpp
)


ADD_EXECUTABLE(mkv ${sources})


