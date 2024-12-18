INCLUDE(admCheckFontConfig)
checkFontConfig()

#CONFIG_FONTCONFIG
#CONFIG_ENCA
#CONFIG_ICONV
#
# CONFIG_ASM
# CONFIG_RASTERIZER
# CONFIG_HARFBUZZ   =>
# CONFIG_FRIBIDI
#
SET(ADM_LIB ADM_libass)
ADD_LIBRARY(${ADM_LIB} STATIC)
TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/x86)
SET(Asm_srcs x86/blend_bitmaps
        x86/blur
        x86/cpuid
        x86/rasterizer
        x86/utils
        x86/x86inc
)
IF(ADM_CPU_X86_64)
  SET(Asm_srcs ${Asm_srcs} x86/be_blur)
ENDIF()
#
#
TARGET_SOURCES(${ADM_LIB} PRIVATE
ass_bitmap.c  ass.c  ass_cache.c  ass_fontconfig.c  ass_library.c   ass_render.c  ass_utils.c
ass_font.c ass_parse.c ass_drawing.c ass_render_api.c ass_strtod.c ass_shaper.c
ass_rasterizer_c.c ass_blur.c ass_font.c ass_rasterizer.c
)
IF(ADM_CPU_X86)
  YASMIFY(asmFiles ${Asm_srcs})
  TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE CONFIG_ASM)
  TARGET_SOURCES(${ADM_LIB} PRIVATE ${asmFiles})
ENDIF()

TARGET_COMPILE_OPTIONS(${ADM_LIB} PRIVATE "-std=gnu99")
#

IF(FONTCONFIG_FOUND)
  TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE CONFIG_FONTCONFIG=1)
ENDIF()

# ?
TARGET_COMPILE_DEFINITIONS(${ADM_LIB} PRIVATE CONFIG_ICONV=1 CONFIG_FREETYPE=1 CONFIG_RASTERIZER=1 CONFIG_FRIBIDI=1)

TARGET_INCLUDE_DIRECTORIES(${ADM_LIB} PUBLIC ${Iconv_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${ADM_LIB} PUBLIC ${Iconv_LIBRARIES} adm_freetype adm_fribidi)
IF(UNIX)
  TARGET_COMPILE_OPTIONS(${ADM_LIB} PRIVATE  -fPIC)
ENDIF()
#
