/***************************************************************************
                          FadeThrough filter
    Algorithm:
        Copyright Mario Klingemann
        Copyright Maxim Shemanarev
        Copyright 2010 Marko Cebokli
        Copyright 2021 szlldm
    Ported to Avidemux:
        Copyright 2021 szlldm
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#define FADEIN

#include "ADM_vidFadeThrough.cpp" 


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoFadeThrough,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSITION,            // Category
                                      "fadeIn",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("fadeThrough","Fade in"),            // Display name
                                      QT_TRANSLATE_NOOP("fadeThrough","Fade in from combination of multiple effects.") // Description
                                  );

