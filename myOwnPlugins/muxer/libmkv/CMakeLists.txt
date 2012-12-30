###################################################
#
#
###################################################
MESSAGE("#####################################")
MESSAGE("Building libmkv.....")
MESSAGE("#####################################")
MESSAGE("")
SET(libmkvsource
      src/attachments.c        src/chapters.c        src/ebml.c        src/ebml.h        src/Makefile.am        src/matroska.c        src/matroska.h        src/md5.c        src/md5.h        src/tags.c        src/tracks.c
)
ADD_LIBRARY(ADMlibmkv STATIC ${libmkvsource})
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include)
