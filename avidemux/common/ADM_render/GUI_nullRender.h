
/***************************************************************************
    \brief Class to handle native (QT/Gtk render)
    \author (C) 2010 by mean  email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef T_null_RENDER_H
#define T_null_RENDER_H
/**
    \fn class nullRender
*/
class nullRender: public VideoRenderBase
{
      public:
                             nullRender( void ) {ADM_info("Starting null renderer\n");};
                             ~nullRender(){ADM_info("Destroying null renderer\n");};
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom) 
                                                            {return true;}
              virtual	bool stop(void)			    {return true;}
              virtual   bool displayImage(ADMImage *pic)    {return true;}
              virtual   bool changeZoom(renderZoom newZoom) {return true;}
              virtual   bool refresh(void)                  {return true;}
              virtual   bool usingUIRedraw(void)            {return false;};
};
#endif





