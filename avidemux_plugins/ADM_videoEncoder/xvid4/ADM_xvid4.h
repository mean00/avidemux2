/***************************************************************************
                          \fn xvid4Encoder
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
#ifndef ADM_xvid4_H
#define ADM_xvid4_H
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "xvid4_encoder.h"
#include "xvid.h"
/**
        \class xvid4Encoder
        \brief Xvid4 mpeg4 encoder

*/
class xvid4Encoder : public ADM_coreVideoEncoder
{
protected:
               
               void           *handle;
               int             plane;
               bool            globalHeader;
               bool            preAmble (ADMImage * in);
               bool            postAmble (ADMBitstream * out,xvid_enc_stats_t *stat,int size);
               bool            query(void);

                xvid_plugin_single_t single;
                xvid_plugin_2pass1_t pass1;
                xvid_plugin_2pass2_t pass2;
                xvid_enc_frame_t xvid_enc_frame;
                xvid_enc_stats_t xvid_enc_stats;
                xvid_enc_plugin_t plugins[7];

                uint32_t        frameNum;
                uint32_t        currentRef;
                uint32_t        backRef;
                uint32_t        fwdRef;
                uint32_t        refIndex;
public:

                           xvid4Encoder(ADM_coreVideoFilter *src,bool globalHeader);
virtual                    ~xvid4Encoder();
virtual        bool        setup(void); 
virtual        bool        encode (ADMBitstream * out);
virtual const  char        *getFourcc(void) {return "DIVX";}

virtual        bool         isDualPass(void) ;
static         int          hook (void *handle, int opt, void *param1, void *param2);

};

#endif
