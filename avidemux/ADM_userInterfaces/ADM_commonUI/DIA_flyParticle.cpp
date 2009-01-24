/***************************************************************************
                          DIA_flyParticle.cpp  -  configuration dialog for
						   particle filter
                              -------------------
                         Chris MacGregor, September 2007
                         chris-avidemux@bouncingdog.com
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



#include "ADM_image.h"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter/ADM_vidParticle.h"
#include "DIA_flyDialog.h"
#include "DIA_flyParticle.h"

/************* COMMON PART *********************/

uint8_t flyParticle::sliderChanged ()
{
    ADM_assert(_yuvBuffer);
    ADM_assert(_rgbBufferOut);
    ADM_assert(_in);
    
    if (preview_mode == PREVIEWMODE_THIS_FILTER)
    {
        uint32_t frame = sliderGet();
        uint32_t len, flags;

        if (!_in->getFrameNumberNoAlloc (frame, &len, _yuvBuffer, &flags))
        {
            printf ("[FlyDialog] Cannot get frame %u\n", frame);
            return 0;
        }
    }
    else if (preview_mode == PREVIEWMODE_LATER_FILTER)
    {
        // No need to call getFrameNumberNoAlloc() here, because our process()
        // (see below) will do it (indirectly, by calling the later filter's
        // getFrameNumberNoAlloc(), which will call the preceding filter's
        // getFrameNumberNoAlloc(), which in turn will call it for the filter
        // before that, etc.):
    }
    else // if (preview_mode == PREVIEWMODE_EARLIER_FILTER)
    {
#if 0
// in this filter, process() has to always reprocess (because we draw the crop
// lines onto the output buffer), so no point in doing it here...

       // In this case, we just call the earlier filter's
        // getFrameNumberNoAlloc() and send the result straight into the
        // output buffer - we needn't do anything to it ourselves.

        uint32_t frame = sliderGet();
        uint32_t len, flags;

        if (!source->getFrameNumberNoAlloc (frame, &len, _yuvBufferOut,
                                            &flags))
        {
            printf ("[FlyDialog] Cannot get frame %u\n", frame);
            return 0;
        }
#endif
    }

    // Process...    
    if (_isYuvProcessing)
    {
        process();
        copyYuvFinalToRgb();
    }
    else // RGB Processing      
    {
        // HERE: we are NOT set up for this to work...
        ADM_assert(0);
        ADM_assert(_rgbBuffer);
        copyYuvScratchToRgb();
        process();
    }

    return display();
}

uint8_t flyParticle::update ()
{
    download();
    pushParam();
    process();
    copyYuvFinalToRgb();
    return display();
}

static inline void draw_hline (uint8_t * fb, uint32_t width, uint32_t height,
                               uint32_t y)
{
    for (uint8_t * pixp = fb + width * y; width--; pixp++)
        *pixp = ~*pixp;
}

static inline void draw_vline (uint8_t * fb, uint32_t width, uint32_t height,
                               uint32_t x)
{
    for (uint8_t * pixp = fb + x; height--; pixp += width)
        *pixp = ~*pixp;
}

uint8_t flyParticle::process ()
{
    uint8_t ret;
    if (preview_mode == PREVIEWMODE_THIS_FILTER)
    {
        ret = ADMVideoParticle::doParticle (_yuvBuffer, _yuvBufferOut,
                                            _in, 0, 0, &param, _w, _h);
    }
    else if (preview_mode == PREVIEWMODE_LATER_FILTER)
    {
        uint32_t len = 0;
        uint32_t flags = 0;
        uint32_t frame = sliderGet();
        ret = source->getFrameNumberNoAlloc (frame, &len, _yuvBufferOut,
                                             &flags);
    }
    else // if (preview_mode == PREVIEWMODE_EARLIER_FILTER)
    {
#if 0
// in this filter, process() has to always reprocess (because we draw the crop
// lines onto the output buffer)...

        // nothing to do here - we've already arranged for the earlier filter
        // to write the result into the output buffer, and changes in our
        // parameters can't affect the result of an earlier filter.

        ret = 1;
#else
        uint32_t len = 0;
        uint32_t flags = 0;
        uint32_t frame = sliderGet();
        ret = source->getFrameNumberNoAlloc (frame, &len, _yuvBufferOut,
                                             &flags);
#endif
    }

    input_buffer_valid = false;

    uint8_t * fb = YPLANE(_yuvBufferOut);

    if (param.left_crop)
        draw_vline (fb, _w, _h, param.left_crop - 1);
    if (param.right_crop)
        draw_vline (fb, _w, _h, _w - param.right_crop);
    if (param.top_crop)
        draw_hline (fb, _w, _h, param.top_crop - 1);
    if (param.bottom_crop)
        draw_hline (fb, _w, _h, _h - param.bottom_crop);

    return ret;
}

/************* COMMON PART *********************/
