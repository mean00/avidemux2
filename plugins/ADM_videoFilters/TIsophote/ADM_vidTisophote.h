/*
**                     TIsophote v0.9.1 for AviSynth 2.5.x
**
**   TIsophote is a simple unconstrained level-set (isophote) smoothing filter.
**
**   Copyright (C) 2004 Kevin Stone
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   This program is distributed in the hope that it will be useful,
**   but WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**   GNU General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef ADM_TISOPHOTE
#define ADM_TISOPHOTE

typedef struct  TISO_CONF
{
    uint32_t        iterations ; //  int 4
    float           tStep;        // float 0.2
    uint32_t        type;         // drop down 0--2 2
    uint32_t        chroma;       // bool 0
}KERNEL_CONF;

class  ADMVideoTIsophote:public AVDMGenericVideoStream
 {

 protected:
        virtual char 		*printConf(void) ;
	TISO_CONF		*_param;
        int			 debug;
	VideoCache		*vidCache;
        ADMImage    *dst1;
        ADMImage    *dst2;


 public:

  			ADMVideoTIsophote(  AVDMGenericVideoStream *in,CONFcouple *setup);
  			~ADMVideoTIsophote();
	virtual uint8_t getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
						ADMImage *data,uint32_t *flags);
	virtual uint8_t	getCoupledConf( CONFcouple **couples)		;
	virtual uint8_t configure( AVDMGenericVideoStream *instream);

};

#endif
