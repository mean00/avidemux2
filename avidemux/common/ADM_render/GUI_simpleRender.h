
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

#ifndef T_SIMPLE_RENDER_H
#define T_SIMPLE_RENDER_H
/**
    \fn class simpleRender
*/
class simpleRender: public VideoRenderBase
{
      protected:
                             GUI_WindowInfo info;
                             uint8_t *videoBuffer;
                             void    *handle;
                             bool    cleanup(void);
                             bool    allocateStuff(void);
      public:
                             simpleRender( void ) ;
                             ~simpleRender();
              virtual	bool init( GUI_WindowInfo *  window, uint32_t w, uint32_t h,renderZoom zoom);
              virtual	bool stop(void);				
              virtual   bool displayImage(ADMImage *pic);
              virtual   bool changeZoom(renderZoom newZoom);
              virtual   bool refresh(void);
};
#endif




