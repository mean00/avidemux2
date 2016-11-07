/** *************************************************************************
             
    \fn Q_encoding.h
    
                      
    copyright            : (C) 2008 by mean/gruntster/?
    
 ***************************************************************************/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef DIA_encoding_none_h
#define DIA_encoding_none_h


#include "ADM_inttype.h"
#include "DIA_encoding.h"
/**
    \class DIA_encodingQt4
*/

class DIA_encodingCli : public DIA_encodingBase
{
public:
    DIA_encodingCli(uint64_t duration);
    ~DIA_encodingCli( );
    
protected:
    void setTotalSize(uint64_t size);
    void setAudioSize(uint64_t size);
    void setVideoSize(uint64_t size);
    void setPercent(uint32_t percent);
    void setFps(uint32_t fps);
    void setFrameCount(uint32_t nb);
    void setElapsedTimeMs(uint32_t nb);
    void setRemainingTimeMS(uint32_t nb);
    void setAverageQz(uint32_t nb);
    void setAverageBitrateKbits(uint32_t kb);

public:    

    
    void setPhasis(const char *n);
    void setAudioCodec(const char *n);
    void setVideoCodec(const char *n);
    void setBitrate(uint32_t br,uint32_t globalbr);
    void setContainer(const char *container);
    void setQuantIn(int size);
    bool isAlive( void );
    
};
#endif	// DIA_encoding_none_h
