/***************************************************************************
            \file            muxerRaw
            \brief           i/f to lavformat mpeg4 muxer
                             -------------------
    
    copyright            : (C) 2008 by mean
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
#include "ADM_default.h"
#include "fourcc.h"
#include "muxerRaw.h"
#include "DIA_coreToolkit.h"


#if 1
#define aprintf(...) {}
#else
#define aprintf printf
#endif


/**
    \fn     muxerRaw
    \brief  Constructor
*/
muxerRaw::muxerRaw() 
{
    file=NULL;
    maxFiles = 1;
    memset(fmt, 0, FMT_BUF_SIZE);
};
/**
    \fn     muxerRaw
    \brief  Destructor
*/

muxerRaw::~muxerRaw() 
{
    ADM_info("Destructing\n");
    close();
}
/**
    \fn open
    \brief Check that the streams are ok, initialize context...
*/

bool muxerRaw::open(const char *fil, ADM_videoStream *s,uint32_t nbAudioTrack,ADM_audioStream **a)
{
    const int pow10arr[6] = {1, 10, 100, 1000, 10000, 100000};
    vStream=s;
    if (muxerConfig.separateFiles)
    {
        ADM_PathSplit(fil, baseName, ext);
        int check = baseName.size();
        int dgts = muxerConfig.maxDigits;
        if (dgts < 2 || dgts > 6)
        {
            ADM_warning("Invalid number of digits %d, defaulting to 4.\n", dgts);
            dgts = 4;
        }

        switch(muxerConfig.extIdx)
        {
            case EXT_DEFAULT:
                break;
            case EXT_BIN:
                ext = "bin";
                break;
            case EXT_JPEG:
                ext = "jpg";
                break;
            default:
                ADM_warning("Invalid output extension index %d, must be less than %d\n", muxerConfig.extIdx, NB_EXT);
                break;
        }

        check += dgts + ext.size() + 2; /* accounting for dot and terminating '\0' */

        if (check > MAX_LEN)
        {
            ADM_error("Full path is too long (%d), aborting.\n", check);
            return false;
        }

        maxFiles = pow10arr[dgts];
        snprintf(fmt, FMT_BUF_SIZE, "%%s-%%0%dd.%s", dgts, ext.c_str());
        snprintf(fullName, MAX_LEN, fmt, baseName.c_str(), 0);
    } else
    {
        if (strlen(fil) >= MAX_LEN)
        {
            ADM_error("Full path is too long (%d), aborting.\n", strlen(fil));
            return false;
        }
        strncpy(fullName, fil, MAX_LEN);
    }

    file = ADM_fopen(fullName, "w");

    if(!file)
    {
        ADM_error("Cannot open \"%s\"\n", fullName);
        return false;
    }
    setOutputFileName(fullName);
    return true;
}

/**
    \fn save
*/
bool muxerRaw::save(void) 
{
    ADM_info("Saving\n");
    uint32_t bufSize=vStream->getWidth()*vStream->getHeight()*3;
    uint8_t *buffer=new uint8_t[bufSize];
    uint64_t lastVideoDts=0;
    int written=0;
    bool result=true;
    ADMBitstream in(bufSize);
    in.data=buffer;
    initUI(QT_TRANSLATE_NOOP("rawmuxer","Saving raw video"));
    encoding->setContainer(QT_TRANSLATE_NOOP("rawmuxer","None"));
    while(true==vStream->getPacket(&in))
    {
        if(in.dts==ADM_NO_PTS)
            in.dts=lastVideoDts+videoIncrement;
        lastVideoDts = in.dts;
        encoding->pushVideoFrame(in.len,in.out_quantizer,in.dts);
        if(updateUI()==false)
        {
            result=false;
            goto abt;
        }
        if (muxerConfig.separateFiles && !file)
        {
            snprintf(fullName, MAX_LEN, fmt, baseName.c_str(), written);
            file = ADM_fopen(fullName, "w");
            if (!file)
            {
                ADM_error("Cannot open \"%s\"\n", fullName);
                result = false;
                goto abt;
            }
        }
        if (!fwrite(buffer,in.len,1,file))
        {
            result = false;
            goto abt;
        }
        written++;
        if (muxerConfig.separateFiles)
        {
            if (written >= maxFiles)
                goto abt;
            fclose(file);
            file = NULL;
        }
    }
abt:
    closeUI();
    delete [] buffer;
    ADM_info("Wrote %d frames \n", written);
    return result;
}
/**
    \fn close
    \brief Cleanup is done in the dtor
*/
bool muxerRaw::close(void) 
{
    if(file)
    {
        fclose(file);
        file=NULL;
    }
    return true;
}

//EOF
