/***************************************************************************
                          audioeng_process.h
                             -------------------
    begin                : 2006
    copyright            : (C) 2002-2006 by mean
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

#ifndef __Audio_ENG_Process__
#define __Audio_ENG_Process__

#include "ADM_coreAudioFilterAPI6_export.h"
#include <string>
#define AUD_PROCESS_BUFFER_SIZE 48000*4*4 // should be enougth 4 seconds of stereo
#include "ADM_coreAudio.h"
/**
  This enumerate is used to give a more accurate error when no audio is output from
  an audio filter.
*/
typedef enum 
{
  AUD_OK=1,           ///< No error
  AUD_ERROR,          ///< No data was caused by an error
  AUD_NEED_DATA,      ///< Filter was stalled, should not happend
  AUD_END_OF_STREAM   ///< End of incoming stream, this is the nominal error case
} AUD_Status;
/*
    incoming --> incomingBuffer  || Processing  --> fill (into next in chain buffer)
          
                   
*/


/**
  This class is the base class for all audio filter.
  Note that all datas are handled as float!
*/
class ADM_COREAUDIOFILTERAPI6_EXPORT AUDMAudioFilter
{
  protected:
    //! This will be used to store data coming from the previous filter
    ADM_floatBuffer   _incomingBuffer;
    //! Pointer to read/write indeces in the _incomingBuffer
    uint32_t        _head;
    uint32_t        _tail;

    //! Describe the output wav format, _wavHeader->byterate holds the size in # of float (NOT SAMPLE)
    WAVHeader       _wavHeader;

    //! Pointer to the previous filter in the chain
    AUDMAudioFilter *_previous;

    //! Fill up the incoming buffer, it is called to pull data from the previous filter
    //! \param status Status of the fill operation
    virtual uint8_t fillIncomingBuffer(AUD_Status *status);
    //! length in float
    uint32_t        _length;
    
  public:
/** Constructor
    \param previous : Pointer to previous in chain filter 
*/
    AUDMAudioFilter(AUDMAudioFilter *previous);
    
//! Destructor                                
    virtual                ~AUDMAudioFilter();
                                
//! Compact the _incomingBuffer to avoid overflow
    uint8_t    shrink( void); 

//! To be called by the next in chain filter. Fills output with processed datas up to max float
//! \param max : Max number of float to put
//! \param output : Where to store output float
//! \param status : Status of the fill operation
    virtual    uint32_t   fill(uint32_t max,float *output,AUD_Status *status)=0;
                                                                                     
//! Returns the output wavheader infos field
        virtual    WAVHeader  *getInfo(void);
        
//! Rewind the stream to the beginning. Used mainly by the normalize filter 
        virtual   uint8_t    rewind(void)  ;
                  uint32_t   getLength(void) {return _length;};
// Returns the channel mapping, by default it is the on from the previous
// The value returned is an array up to MAX_CHANNELS                  
        virtual   CHANNEL_TYPE    *getChannelMapping(void ) {return _previous->getChannelMapping();}
        virtual const std::string &getLanguage(void)
        {
            return _previous->getLanguage();
        }
};
#endif
