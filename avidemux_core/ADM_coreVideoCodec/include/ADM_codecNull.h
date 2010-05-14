/***************************************************************************
                          ADM_codecNull.h  -  description
                             -------------------
    begin                : Fri Apr 19 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
class decoderNull:public decoders
{
protected:

public:
  decoderNull (uint32_t w, uint32_t h,uint32_t fcc, uint32_t extraDataLen, uint8_t *extraData,uint32_t bpp)
        :decoders (  w,   h,  fcc,   extraDataLen,  extraData,  bpp)
  {
  }
  virtual ~ decoderNull ()
  {
  };
  virtual bool uncompress (ADMCompressedImage * in, ADMImage * out)
  {
    for(int i=PLANAR_Y;i<PLANAR_LAST;i++)
    {
        uint32_t pitch=out->GetPitch((ADM_PLANE)i);
        uint32_t line=_w;
        uint32_t colmn=_h;
        uint8_t *dest=out->GetWritePtr((ADM_PLANE)i);
        if(i!=PLANAR_Y)
        {
            line>>=1;
            colmn>>=1;
        }
        uint8_t *src;
        int plane=_w*_h;
        switch(i)
        {
             case PLANAR_Y: src=in->data;break;
             case PLANAR_U: src=in->data+plane;break;
             case PLANAR_V: src=in->data+((plane*5)>>2);break;
        }
        BitBlit(dest, pitch,src,line,line,colmn);
    }
    uint64_t pts,dts;
    pts=in->demuxerPts;
    dts=in->demuxerDts;
    if(pts!=ADM_COMPRESSED_NO_PTS)
    {
        out->Pts=pts;
    }else
    {
        out->Pts=dts;
    }
    return 1;
  }
};
