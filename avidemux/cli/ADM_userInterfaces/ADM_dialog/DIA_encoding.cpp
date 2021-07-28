/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <math.h>
#include "ADM_inttype.h"
//#include "Q_encoding.h"

#include "prefs.h"
#include "DIA_working.h"
#include "DIA_encoding.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "DIA_encoding_none.h"

extern bool ADM_slaveReportProgress(uint32_t p);
DIA_encodingCli::DIA_encodingCli(uint64_t fps1000) : DIA_encodingBase(fps1000)
{
    firstPass=false;
}


DIA_encodingCli::~DIA_encodingCli( )
{
    // TODO: optionally delete first pass log files
}
void DIA_encodingCli::setPhase(ADM_ENC_PHASE_TYPE phase, const char *n)
{
    const char *desc = n;
    firstPass = phase == ADM_ENC_PHASE_FIRST_PASS;
    if(firstPass && !desc)
        desc = "Pass 1";

    fprintf(stderr,"Encoding Phase        : %s\n",desc);
}
void DIA_encodingCli::setLogFileName(const char *name)
{
    if(!name)
    {
        logFileName.clear();
        return;
    }
    logFileName = name;
}
void DIA_encodingCli::setPercent(uint32_t percent)
{
    printf("%*" PRIu32"%% done%s",3,percent,firstPass? "\n" : "\t");
    ADM_slaveReportProgress(percent);
}
void DIA_encodingCli::setAudioCodec(const char *n)
{
            fprintf(stderr,"Encoding Audio codec  : %s\n",n);

}

void DIA_encodingCli::setContainer(const char *container)
{
        fprintf(stderr,"Encoding Container        : %s\n",container);
}
#define  ETA_SAMPLE_PERIOD 60000 //Use last n millis to calculate ETA
#define  GUI_UPDATE_RATE 500  

         
bool DIA_encodingCli::isAlive( void )
{

        return 1;
}
    void DIA_encodingCli::setTotalSize(uint64_t size) { UNUSED_ARG(size); }
    void DIA_encodingCli::setAudioSize(uint64_t size) { UNUSED_ARG(size); }
    void DIA_encodingCli::setVideoSize(uint64_t size) { UNUSED_ARG(size); }
    void DIA_encodingCli::setFps(uint32_t fps) { UNUSED_ARG(fps); }
    void DIA_encodingCli::setFrameCount(uint32_t nb) { printf("frames: %" PRIu32"\t",nb); }
    void DIA_encodingCli::setElapsedTimeMs(uint32_t nb) { printf("elapsed: %s\n",ADM_us2plain(1000LL*nb)); }
    void DIA_encodingCli::setRemainingTimeMS(uint32_t nb) { UNUSED_ARG(nb); }
    void DIA_encodingCli::setAverageQz(uint32_t nb) { UNUSED_ARG(nb); }
    void DIA_encodingCli::setAverageBitrateKbits(uint32_t kb) { UNUSED_ARG(kb); }
    void DIA_encodingCli::setVideoCodec(const char *n) { UNUSED_ARG(n); }
//**********************************
namespace ADM_CliCoreUIToolkit
{
extern DIA_encodingBase *createEncoding(uint64_t duration)
{
        return new DIA_encodingCli(duration);
}

}



