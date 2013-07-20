/***************************************************************************
                          DIA_factoryStubs.h
  
  (C) Mean 2008 fixounet@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_UITYPES_H
#define DIA_UITYPES_H

#define ADM_UI_CLI 1
#define ADM_UI_GTK 2
#define ADM_UI_QT4 4
#define ADM_UI_NONE 8
#define ADM_UI_MASK 15

#define ADM_FEATURE_VDPAU  32
#define ADM_FEATURE_LIBVA  64
#define ADM_FEATURE_OPENGL 128

#define ADM_FEATURE_MASK   (ADM_FEATURE_VDPAU+ADM_FEATURE_LIBVA+ADM_FEATURE_OPENGL)


#define ADM_UI_ALL (ADM_UI_CLI+ADM_UI_GTK+ADM_UI_QT4)
typedef  int  ADM_UI_TYPE;

#endif

