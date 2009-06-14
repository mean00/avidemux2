
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VOBSUB_BITMAP_H
#define _VOBSUB_BITMAP_H
//************************************
class vobSubBitmap
{
  protected:
        uint8_t                         *_dirty;                   /// Dirty lines (non transparent)
  public:
        uint8_t                         isDirty(uint32_t line);
        uint8_t                         setDirty(uint32_t line);

        uint32_t                        _width,_height;
        uint32_t                        placeTop, placeHeight;     /// Position of the sub
      
        uint8_t                         *_bitmap;                  /// YUV image
        uint8_t                         *_alphaMask;               /// alpha mask 
        
 
                                        vobSubBitmap(uint32_t w, uint32_t h); 
                                        ~vobSubBitmap();
        void                            clear(void);
        
                                        /// Convert palette bitmap to yuv&mask bitmap
        uint8_t                         buildYUV( int16_t *palette ); 
                                        /// Generate the final bitmap (resized)
        uint8_t                         subResize(vobSubBitmap **tgt,uint32_t newx,uint32_t newy,
                                                uint32_t oldtop, uint32_t oldheight);
};

#endif
