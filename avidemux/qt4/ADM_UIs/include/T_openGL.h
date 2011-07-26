/***************************************************************************
  \file T_openGL.h
  \brief OpenGL related filters
  \author (C) 2011 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef T_OPENGL_H
#define T_OPENGL_H
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#	include <OpenGL/gl.h>
#	include <OpenGL/glext.h>
#	define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#else
#	include <GL/gl.h>
#	include <GL/glext.h>
#endif

#include <QtOpenGL/QtOpenGL>


// Get glActiveTexture
typedef GLAPI void APIENTRY (GlActiveTexture_Type)(GLenum texture);
bool ADM_setActiveTexture(GlActiveTexture_Type *set);
GlActiveTexture_Type *ADM_getActiveTexture(void);
#endif

