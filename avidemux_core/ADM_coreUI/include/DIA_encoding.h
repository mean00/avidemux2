/**********************************************************************
            \file            DIA_encoding.h

    
        
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_ENCODING_H
#define ADM_ENCODING_H


#define MAX_BR_SLOT 200

typedef struct 
{
  uint32_t size;
  uint32_t quant;
}encodingSlice;

class DIA_encodingBase
{
protected:
                Clock	clock;
                uint32_t  _lastTime;            // Start time used to calc. ETA
                uint32_t  _lastFrame;           // Start frame used to calc. ETA
                uint32_t  _nextSampleStartTime; // Next start time to be used for ETA
                uint32_t  _nextSampleStartFrame; // Next start frame for ETA
                uint32_t  _nextUpdate;           // Next time to update the GUI
                float _fps_average;
                uint32_t _average_bitrate;
                uint64_t _totalSize;
                uint64_t _audioSize;
                uint64_t _videoSize;
                uint32_t _bitrate_sum;           // Sum of bitrate array
                encodingSlice _bitrate[MAX_BR_SLOT];
                uint32_t _roundup;
                uint32_t _current;
                uint32_t _total;
                uint32_t _lastnb;
                uint32_t _fps1000;
                uint32_t _originalPriority;
        
public:
                             DIA_encodingBase( uint32_t fps1000 );
                virtual      ~DIA_encodingBase( );
                
                virtual void reset( void );
                virtual void setPhasis(const char *n);
                virtual void setCodec(const char *n);
                virtual void setAudioCodec(const char *n);
                virtual void setFps(uint32_t fps);
                virtual void setFrame(uint32_t nb,uint32_t size, uint32_t quant,uint32_t total);
                virtual void setPercent(uint32_t percent);
                virtual void setContainer(const char *container);
                virtual void setAudioSize(uint32_t size);
                virtual uint8_t isAlive(void);
};
//********************
DIA_encodingBase *createEncoding(uint32_t fps1000);
#endif
