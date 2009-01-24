/***************************************************************************
                            ADM_externalEncoder.h

    begin                : Tue Apr 15 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ADM_externalEncoder_H
#define ADM_externalEncoder_H

#include "ADM_codecs/ADM_codec.h"
#include "ADM_videoFilter.h"
#include "ADM_vidEncode.hxx"
#include "adm_encoder.h"
#include "ADM_pluginLoad.h"

class externalEncoder : public Encoder
{
private:
	ADM_vidEnc_plugin *_plugin;
	bool _openPass;
	char* _logFileName;
	bool _globalHeader;
	uint8_t *_extraData;
	int _extraDataSize;

	uint8_t startPass(void);

public:
	externalEncoder(COMPRES_PARAMS *params, bool globalHeader);
	~externalEncoder();
	virtual uint8_t isDualPass(void);
	virtual uint8_t configure(AVDMGenericVideoStream * instream, int useExistingLogFile);
	virtual uint8_t encode(uint32_t frame, ADMBitstream *out);
	virtual uint8_t hasExtraHeaderData(uint32_t *l, uint8_t **data);
	virtual uint8_t setLogFile(const char *p, uint32_t fr);
	virtual uint8_t stop(void);
	virtual uint8_t startPass1(void);
	virtual uint8_t startPass2(void);
	virtual const char *getCodecName(void);
	virtual const char *getFCCHandler(void);
	virtual const char *getDisplayName(void);
};
#endif	// ADM_externalEncoder_H
