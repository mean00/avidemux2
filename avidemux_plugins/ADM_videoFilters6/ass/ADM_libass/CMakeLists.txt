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
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
SET(ADM_LIB ADM_libass)
#  mputils.c
IF(ADM_CPU_X86) # We have yasm, we use it to build ffmpeg. Borrowed from x265
        find_program(YASM_EXECUTABLE 
        NAMES yasm yasm-1.2.0-win32 yasm-1.2.0-win64
        )
        MESSAGE(STATUS "Yasm found as ${YASM_EXECUTABLE}")
        enable_language(ASM_YASM)
        SET(Asm x86/blend_bitmaps.asm  
                x86/blur.asm    x86/cpuid.asm  
                x86/rasterizer.asm  x86/utils.asm  
                x86/x86inc.asm
        )
        IF(ADM_CPU_X86_64)
        SET(Asm ${Asm} x86/be_blur.asm )
        ENDIF(ADM_CPU_X86_64)
        ADD_DEFINITIONS(-DCONFIG_ASM) 
ENDIF(ADM_CPU_X86)
#
SET(${ADM_LIB}_SRCS 
ass_bitmap.c  ass.c  ass_cache.c  ass_fontconfig.c  ass_library.c   ass_render.c  ass_utils.c
ass_font.c ass_parse.c ass_drawing.c ass_render_api.c ass_strtod.c ass_shaper.c
ass_rasterizer_c.c ass_blur.c ass_font.c 
${Asm}
)

IF(WIN32 AND NOT ADM_CPU_X86_64) # This is wrong, FIXME
        SET(${ADM_LIB}_SRCS  ${${ADM_LIB}_SRCS} ass_rasterizer.c)
ENDIF(WIN32 AND NOT ADM_CPU_X86_64)
#
ADD_DEFINITIONS(" -std=gnu99 ")
#
ADD_LIBRARY(${ADM_LIB} STATIC ${${ADM_LIB}_SRCS})
ADD_DEFINITIONS(${FREETYPE2_CFLAGS} ${FRIDIBI_CFLAGS})
INCLUDE_DIRECTORIES(${LIBICONV_INCLUDE_DIR})

IF (FONTCONFIG_FOUND)
	ADD_DEFINITIONS(${FONTCONFIG_CFLAGS} "-DCONFIG_FONTCONFIG=1")
ENDIF (FONTCONFIG_FOUND)

# ?
ADD_DEFINITIONS(${FONTCONFIG_CFLAGS} "-DCONFIG_ICONV=1 -DCONFIG_FREETYPE=1 -DCONFIG_RASTERIZER=1 -DCONFIG_FRIBIDI=1")
TARGET_LINK_LIBRARIES(${ADM_LIB} ${LIBICONV_LIBRARY_DIR})

IF (UNIX)
	ADD_TARGET_CFLAGS(${ADM_LIB} -fPIC)
ENDIF (UNIX)
