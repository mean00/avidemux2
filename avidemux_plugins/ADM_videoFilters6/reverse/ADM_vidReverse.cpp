/** *************************************************************************
                    \fn       ADM_vidReverse.cpp  

    copyright: 2022 szlldm

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
#include "ADM_coreVideoFilter.h"
#include "DIA_factory.h"
#include "ADM_vidMisc.h"

#include "reverse.h"
#include "reverse_desc.cpp"
#include "ADM_vidReverse.h"


// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ADMVideoReverse,   // Class
                        1,0,1,              // Version
                        ADM_UI_TYPE_BUILD,         // UI
                        VF_TRANSFORM,            // Category
                        "reverse",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("reverse","Reverse"),            // Display name
                        QT_TRANSLATE_NOOP("reverse","Play a short section backward.") // Description
                    );

#if 1
    #define aprintf(...) {}
#else
    #define aprintf printf
#endif

/**
    \fn ADMVideoReverse
    \brief constructor
*/
ADMVideoReverse::ADMVideoReverse(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
UNUSED_ARG(setup);

    original=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    if(!setup || !ADM_paramLoad(setup,reverse_param,&param))
    {
        // Default value
        param.startTime = info.markerA / 1000LL;
        param.endTime = info.markerB / 1000LL;
        param.fileBuffer = "";
    }
    frameCount = 0;
    overrun = false;
    overrunImg = new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    invalidatedBySeek = false;
    internalError = false;
    bufferCount = 0;
    frameBuffer = NULL;
    validFileBuffer = false;
}
/**
    \fn ADMVideoReverse
    \brief destructor
*/
ADMVideoReverse::~ADMVideoReverse()
{
    if(original) delete original;
    original=NULL;
    if(overrunImg) delete overrunImg;
    overrunImg=NULL;
    clean();
    cleanFile();
}


/**
 * \fn clean
 */
void ADMVideoReverse::clean()
{
    frameCount = 0;
    if (bufferCount > 0)
    {
        free(frameBuffer);
        frameBuffer = NULL;
        bufferCount = 0;
    }
}

/**
 * \fn cleanFile
 */
void ADMVideoReverse::cleanFile()
{
    validFileBuffer = false;
    FILE * fd = NULL;
    fd = ADM_fopen(param.fileBuffer.c_str(), "w+");
    if(fd != NULL)
        ADM_fclose(fd);
}

/**
 * \fn goToTime
 */
bool ADMVideoReverse::goToTime(uint64_t time)
{
    uint32_t timeMs = time / 1000;
    if ((timeMs >= param.startTime) && (timeMs <= param.endTime))
    {
        invalidatedBySeek = (timeMs > param.startTime);	// special case is if seek to the start, we need to handle it, because if reverse start at the first frame, this can happen!
        overrun = false;
        clean();
        if (timeMs > param.startTime)
            cleanFile();
    }
    
    return previousFilter->goToTime(time);
}

/**
 * \fn wrongImage
 */
void ADMVideoReverse::wrongImage(ADMImage * img, int Y, int U, int V, const char * msg)
{
    uint32_t w, h;
    img->getWidthHeight(&w, &h);
    uint8_t * wplanes[3];
    int strides[3];
    img->GetWritePlanes(wplanes);
    img->GetPitches(strides);
    memset(wplanes[0], Y, h*strides[0]);
    memset(wplanes[1], U, (h/2)*strides[1]);
    memset(wplanes[2], V, (h/2)*strides[2]);
    img->printString(1,1,msg);
}

/**
    \fn getFrame
    \brief Get a processed frame
*/
bool ADMVideoReverse::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if (internalError)
    {
        internalError = false;
        clean();
        cleanFile();
        return false;
    }
    
    if (frameCount == 0)
    {
        clean();
        if (overrun)
        {
            image->duplicateFull(overrunImg);
            overrun = false;
            *fn = _fn;
            aprintf("[reverse] getNextFrame (fn==%u) - Return overrun\n",*fn);
            return true;
        } else {
            if(false==previousFilter->getNextFrame(fn,original))
            {
                ADM_warning("reverse : Cannot get frame\n");
                return false;
            }
            
            uint32_t startMs = param.startTime;
            uint32_t endMs = param.endTime;
            

            if (startMs > endMs)
            {
                uint64_t tmp = endMs;
                endMs = startMs;
                startMs = tmp;
            }
            
            uint32_t imgMs = (original->Pts+getAbsoluteStartTime())/1000LL;
            
            if ((startMs == endMs) || (imgMs < startMs) || (imgMs > endMs))
            {
                invalidatedBySeek = false;
                image->duplicateFull(original);
                aprintf("[reverse] getNextFrame (fn==%u) - out of scope\n",*fn);
                return true;
            }
            
            
            if ((imgMs > startMs+1) && (imgMs <= endMs))
            {
                invalidatedBySeek = true;
                clean();
            }
            
            if (invalidatedBySeek)
            {
                image->copyInfo(original);
                // make it green
                wrongImage(image, 64, 0, 0, "[reverse] preview not available");
                aprintf("[reverse] getNextFrame (fn==%u) - invalidated by seek\n",*fn);
                return true;
            }
            
            _fn = *fn;
            
            // we are in the time scope
            // get and buffer frames
            
            FILE * fd = NULL;
            if (!validFileBuffer)
            {
                fd = ADM_fopen(param.fileBuffer.c_str(), "w+");
                if (fd == NULL)
                {
                    internalError = true;
                    *fn = _fn;
                    image->copyInfo(original);
                    ADM_error("[reverse] file open error\n");
                    wrongImage(image, 64, 255, 0, "[reverse] file open error\n");
                    return true;
                }
            }
            
            do {
                imgMs = (original->Pts+getAbsoluteStartTime())/1000LL;
                
                if (imgMs > endMs)	// reached out of scope frame
                {
                    overrunImg->duplicateFull(original);
                    overrun = true;
                    break;
                }

                bufferCount = frameCount+1;
                frameBuffer = (buffered_frame_t*)realloc(frameBuffer, bufferCount*sizeof(buffered_frame_t));
                if (frameBuffer == NULL)
                {
                    internalError = true;
                    *fn = _fn;
                    image->copyInfo(original);
                    ADM_error("[reverse] realloc error\n");
                    wrongImage(image, 64, 255, 0, "[reverse] realloc error\n");
                    if (fd)
                        ADM_fclose(fd);
                    return true;
                }

                frameBuffer[frameCount].Pts = (original->Pts+getAbsoluteStartTime());
                frameBuffer[frameCount]._colorSpace = original->_colorSpace;
                frameBuffer[frameCount]._range = original->_range;
                
                uint32_t w,h;
                uint8_t * wplanes[3];
                int strides[3];
                original->getWidthHeight(&w, &h);
                original->GetWritePlanes(wplanes);
                original->GetPitches(strides);
                
                if (!validFileBuffer)
                {
                    size_t count;
                    for (int i=0; i<3; i++)
                    {
                        if (i==1)
                            h /= 2;
                        count = strides[i]*h;
                        if (ADM_fwrite(wplanes[i], 1, count, fd) != count)
                        {
                            internalError = true;
                            *fn = _fn;
                            image->copyInfo(original);
                            ADM_error("[reverse] file write error\n");
                            wrongImage(image, 64, 255, 0, "[reverse] file write error\n");
                            if (fd)
                                ADM_fclose(fd);
                            return true;
                        }
                    }
                }


                frameCount += 1;
                if(false==previousFilter->getNextFrame(fn,original))	// reached the end
                {
                    break;
                }
            } while(1);
            
            if (fd)
            {
                validFileBuffer = true;
                ADM_fclose(fd);
            }
        }
    }
    
    if (frameCount == 0)	// this should be not true, but in case it is, return false
    {
        clean();
        if (overrun)
        {
            image->duplicateFull(overrunImg);
            overrun = false;
            *fn = _fn;
            aprintf("[reverse] getNextFrame (fn==%u) - Return overrun 2\n",*fn);
            return true;
        }
        aprintf("[reverse] getNextFrame (fn==%u) - Return from impossible\n",*fn);
        return false;
    }


    // serve the reverse ordered frames:
    
    frameCount -= 1;

    uint64_t startPts = param.startTime;
    uint64_t endPts = param.endTime;
    if (startPts > endPts)
    {
        uint64_t tmp = endPts;
        endPts = startPts;
        startPts = tmp;
    }
    startPts *= 1000;
    endPts *= 1000;

    if (endPts < frameBuffer[frameCount].Pts)	// due to rounding errors
        image->Pts = 0;
    else
        image->Pts = (endPts - frameBuffer[frameCount].Pts)+startPts;
    if (image->Pts < getAbsoluteStartTime())
        image->Pts = 0;
    else
        image->Pts -= getAbsoluteStartTime();
    image->_colorSpace = frameBuffer[frameCount]._colorSpace;
    image->_range = frameBuffer[frameCount]._range;
    
    uint32_t w,h;
    uint8_t * wplanes[3];
    int strides[3];
    image->getWidthHeight(&w, &h);
    image->GetWritePlanes(wplanes);
    image->GetPitches(strides);
    
    FILE * fd = ADM_fopen(param.fileBuffer.c_str(), "r");
    if (fd == NULL)
    {
        internalError = true;
        *fn = _fn;
        ADM_error("[reverse] file open error\n");
        wrongImage(image, 64, 255, 0, "[reverse] file open error\n");
        return true;
    }

    long int offset = (strides[0]*h + strides[1]*(h/2) + strides[2]*(h/2));
    offset *= frameCount;
    if (fseek(fd, offset, SEEK_SET) != 0)
    {
        internalError = true;
        *fn = _fn;
        ADM_error("[reverse] file seek error\n");
        wrongImage(image, 64, 255, 0, "[reverse] file seek error\n");
        if (fd)
            ADM_fclose(fd);
        return true;
    }

    size_t count;
    for (int i=0; i<3; i++)
    {
        if (i==1)
            h /= 2;
        count = strides[i]*h;
        if (ADM_fread(wplanes[i], 1, count, fd) != count)
        {
            internalError = true;
            *fn = _fn;
            ADM_error("[reverse] file read error\n");
            wrongImage(image, 64, 255, 0, "[reverse] file read error\n");
            if (fd)
                ADM_fclose(fd);
            return true;
        }
    }

    ADM_fclose(fd);

    *fn = _fn;
    _fn += 1;
    aprintf("[reverse] getNextFrame (fn==%u) - Backward\n",*fn);
    return true;
}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         ADMVideoReverse::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, reverse_param,&param);
}

void ADMVideoReverse::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, reverse_param, &param);
    clean();
    cleanFile();
    aprintf("[reverse] setCoupledConf\n");
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *ADMVideoReverse::getConfiguration(void)
{
    static char s[2512];
    
    char startTimeStr[128];
    char endTimeStr[128];
    snprintf(startTimeStr,127,"%s",ADM_us2plain(param.startTime*1000LL));
    snprintf(endTimeStr,127,"%s",ADM_us2plain(param.endTime*1000LL));
    
    uint32_t durationMs = (param.endTime - param.startTime);
    double bytesPerSec;
    bytesPerSec = previousFilter->getInfo()->width;
    bytesPerSec *= previousFilter->getInfo()->height;
    bytesPerSec *= 1.5;	// YUV420
    double fps = 1000000.0 / (double)(previousFilter->getInfo()->frameIncrement);	//frameIncrement: Average delta time between 2 frames in useconds ~ 1/fps
    bytesPerSec *= fps;
    bytesPerSec *= 1.1;	///margin
    double gib = ((bytesPerSec * durationMs) / 1000.0) / 1073741824.0;

    snprintf(s,2511,"%s - %s: Buffer to %s\nRequired storage for buffering: %.3f GiB",startTimeStr,endTimeStr,param.fileBuffer.c_str(),gib);

    return s;
}
/**
    \fn getInfo
*/
FilterInfo  *ADMVideoReverse::getInfo(void)
{
    return &info;
}

extern bool         DIA_reverse(reverse *param, ADM_coreVideoFilter *in);
/**
    \fn configure

*/
bool ADMVideoReverse::configure(void) 
{
    aprintf("[reverse] configure\n");
    clean();
    cleanFile();
    return DIA_reverse(&param, previousFilter);
}

//EOF
