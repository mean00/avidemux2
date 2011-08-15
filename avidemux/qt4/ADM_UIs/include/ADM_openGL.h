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
#ifndef ADM_OPENGL_H
#define ADM_OPENGL_H
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#	include <OpenGL/gl.h>
#	include <OpenGL/glext.h>
#	define GL_TEXTURE_RECTANGLE_NV GL_TEXTURE_RECTANGLE_EXT
#else
#	include <GL/gl.h>
#	include <GL/glext.h>
#endif

#include <QtGui/QImage>
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLShader>

#endif

