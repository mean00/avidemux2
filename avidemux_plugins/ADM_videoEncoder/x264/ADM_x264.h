/***************************************************************************
                          \fn x264Encoder
                          \brief Internal handling of video encoders
                             -------------------
    
    copyright            : (C) 2002/2010 by mean/gruntster
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
#ifndef ADM_x264_H
#define ADM_x264_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "x264_encoder.h"
extern "C"
{
#include "x264.h"
};
/**
        \class x264Encoder
        \brief x264 mpeg4 encoder

*/
class x264Encoder : public ADM_coreVideoEncoder
{
protected:
               x264_param_t     param;
               x264_t          *handle;
               x264_picture_t  pic;
               int             plane;
               bool            globalHeader;
               bool            preAmble (ADMImage * in);
               bool            postAmble (ADMBitstream * out,uint32_t nbNals,x264_nal_t *nal,x264_picture_t *picout);
               bool            createHeader(void);
               int             encodeNals(uint8_t *buf, int size, x264_nal_t *nals, int nalCount, bool skipSei);
               uint32_t        extraDataLen;
               uint8_t         *extraData;
               uint32_t        seiUserDataLen;
               uint8_t         *seiUserData ;
               bool            firstIdr;
               uint32_t        passNumber;
               char            *logFile;

               
public:

                           x264Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~x264Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "H264";}
virtual        bool         getExtraData(uint32_t *l,uint8_t **d) {*l=extraDataLen;*d=extraData;return true;}
virtual        bool         isDualPass(void) ;

virtual        bool         setPassAndLogFile(int pass,const char *name);


};

#endif
