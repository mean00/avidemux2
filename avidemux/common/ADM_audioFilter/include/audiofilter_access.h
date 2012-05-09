/***************************************************************************
            \file audiofilter_access.h
            \brief convert audiofilter to audioaccess (used for playback for example)
            (C) Mean 2009 fixounet@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef AUDM_ACCESS_H
#define AUDM_ACCESS_H

#include "ADM_audioStream.h"
#include "audioencoder.h"
class EditableAudioTrack;
/**
    \class ADMAudioFilter_Access
    \brief Bridge audioFilter->Access

*/
class ADMAudioFilter_Access : public ADM_audioAccess
{
  protected:
    uint64_t            startTimeUs; /*< Starting time in us */
    AUDMAudioFilter     *filter;
    uint64_t            samplesSeen;
    ADM_AudioEncoder    *encoder;
    EditableAudioTrack  *editable; // source container, needed for cleanup
  public:
                WAVHeader         *getWavHeader(void) {return encoder->getInfo();}

                                    ADMAudioFilter_Access(AUDMAudioFilter *incoming,ADM_AudioEncoder *encoder,
                                                                EditableAudioTrack *ed,uint64_t timeUs) ;
                virtual           ~ADMAudioFilter_Access();
                                    /// Return true if the demuxer can seek in time
                virtual bool      canSeekTime(void) {return false;};
                                    /// Return true if the demuxer can seek by offser
                virtual bool      canSeekOffset(void) {return true;};
                                    /// Return true if we can have the audio duration
                virtual bool      canGetDuration(void) {return false;};
                                    /// Returns length in bytes of the audio stream
                virtual uint32_t  getLength(void){return 0;}
                                    /// Set position in bytes
                virtual bool      setPos(uint64_t pos);
                                    /// Get position in bytes
                virtual uint64_t  getPos(void);
                                    /// Grab extra data
                virtual bool      getExtraData(uint32_t *l, uint8_t **d);

                virtual bool    getPacket(uint8_t *buffer, uint32_t *size, uint32_t maxSize,uint64_t *dts) __attribute__((force_align_arg_pointer));
                virtual bool    isCBR(void);
};


#endif

