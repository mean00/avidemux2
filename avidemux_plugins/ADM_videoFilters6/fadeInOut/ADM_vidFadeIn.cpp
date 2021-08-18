/***************************************************************************
                          FadeInOut filter
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
#include "ADM_vidFadeInOut.cpp"

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoFadeInOut,   // Class
                                      1,0,0,              // Version
                                      ADM_UI_TYPE_BUILD,         // UI
                                      VF_TRANSITION,            // Category
                                      "fadeIn",            // internal name (must be uniq!)
                                      QT_TRANSLATE_NOOP("fadeInOut","Fade in"),            // Display name
                                      QT_TRANSLATE_NOOP("fadeInOut","Fade in from color.") // Description
                                  );



