#ifndef ADM_CONFIG_H
#define ADM_CONFIG_H

#include "ADM_coreConfig.h"

#define ADM_BUILD_CLI 1
#define ADM_BUILD_GTK 2
#define ADM_BUILD_QT4 3

/* Jog Shuttle */
#cmakedefine USE_JOG

#define PACKAGE   "avidemux"
#define ADMLOCALE "${ADM_LOCALE}"


#if ${CONFIG_HEADER_TYPE} == ADM_BUILD_CLI || ${CONFIG_HEADER_TYPE} == ADM_BUILD_GTK
/* Define if the GNU gettext() function is already present or preinstalled. */
#cmakedefine HAVE_GETTEXT
#endif

// GTK+ uses X11 framework
#cmakedefine HAVE_GTK_X11

/* stricter prototyping */
#cmakedefine ICONV_NEED_CONST

/* FFmpeg */
#define USE_FFMPEG

/* FreeType2 detected */
#cmakedefine USE_FREETYPE

/* libvpx is available */
#cmakedefine USE_VPX

/* use libsamplerate */
#cmakedefine USE_SRC

#if ${CONFIG_HEADER_TYPE} == ADM_BUILD_GTK ||  ${CONFIG_HEADER_TYPE} == ADM_BUILD_QT4
/* SDL detected */
#cmakedefine USE_SDL
#endif

#if ${CONFIG_HEADER_TYPE} == ADM_BUILD_GTK || ${CONFIG_HEADER_TYPE} == ADM_BUILD_QT4
/* XVideo detected */
#cmakedefine USE_XV
#endif

/* Version number of package */
#define VERSION "${VERSION}"

// FIXME - start
#ifdef HAVE_GETTEXT
#  include <libintl.h>
#  undef _
#endif


/// FIXME - end
#if ${CONFIG_HEADER_TYPE} == ADM_BUILD_QT4
/* OpenGL detected */
#cmakedefine USE_OPENGL
#endif


#endif
