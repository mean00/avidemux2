# VS Style management of ffmpeg
# We raw import binaries for somewhere else as they have been compiled with mingw
#
# Add -DIMPORT_FOLDER=foobar on cmake

IF(NOT IMPORT_FOLDER)
        SET(IMPORT_FOLDER "d:\\import\\")
ENDIF(NOT IMPORT_FOLDER)
MESSAGE(STATUS "Importing ffmpeg binaries from ${IMPORT_FOLDER}")

SET(LIB_IMPORT_FOLDER "${IMPORT_FOLDER}\\lib")
INCLUDE_DIRECTORIES(${IMPORT_FOLDER}/include)
LINK_DIRECTORIES(${IMPORT_FOLDER}/lib)
set(LIBAVCODEC_LIB ${IMPORT_FOLDER}/lib/libavcodec.dll.a)
set(LIBAVFORMAT_LIB ${IMPORT_FOLDER}/lib/libavformat.dll.a)
set(LIBAVUTIL_LIB ${IMPORT_FOLDER}/lib/libavutil.dll.a)
set(LIBPOSTPROC_LIB ${IMPORT_FOLDER}/lib/libpostproc.dll.a)
set(LIBSWSCALE_LIB ${IMPORT_FOLDER}/lib/libswscale.dll.a)


add_library(ADM_libswscale UNKNOWN IMPORTED)
add_library(ADM_libpostproc UNKNOWN IMPORTED)
add_library(ADM_libavutil UNKNOWN IMPORTED)
add_library(ADM_libavcodec UNKNOWN IMPORTED)
add_library(ADM_libavformat UNKNOWN IMPORTED)


set_property(TARGET ADM_libswscale PROPERTY IMPORTED_LOCATION "${LIBSWSCALE_LIB}")
set_property(TARGET ADM_libpostproc PROPERTY IMPORTED_LOCATION "${LIBPOSTPROC_LIB}")
set_property(TARGET ADM_libavutil PROPERTY IMPORTED_LOCATION "${LIBAVUTIL_LIB}")
set_property(TARGET ADM_libavcodec PROPERTY IMPORTED_LOCATION "${LIBAVCODEC_LIB}")
set_property(TARGET ADM_libavformat PROPERTY IMPORTED_LOCATION "${LIBAVFORMAT_LIB}")



# EOF
