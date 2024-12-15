SET(ADM_LIB ADM_coreImage6)
ADM_ADD_SHARED_LIBRARY(${ADM_LIB})
TARGET_SOURCES(${ADM_LIB} PRIVATE
  ADM_image.cpp
  ADM_imageUtils.cpp
  ADM_imageResizer.cpp
  ADM_colorspace.cpp
  ADM_pp.cpp
  ADM_print.cpp
  ADM_imageSave.cpp
  ADM_imageOperation.cpp
  ADM_toneMapper.cpp
)
YASMIFY(bins ADM_imageUtils_asm)
TARGET_SOURCES(${ADM_LIB} PRIVATE ${bins})

TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_core6 )
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_coreUI6 ADM_libswscale ADM_libpostproc ADM_coreUtils6)
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ADM_libavcodec ADM_libavutil)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# ?? ADD_SOURCE_CFLAGS(DIA_flyDialog.cpp " -DADM_UI_TYPE_BUILD=99")

IF(APPLE)
  IF(NOT ADM_CPU_X86_64)
    TARGET_LINK_LIBRARIES(ADM_coreImage6 PRIVATE -Wl,-read_only_relocs,suppress)
  ENDIF()
ENDIF()

ADM_INSTALL_LIB(ADM_coreImage6)
