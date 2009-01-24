/***************************************************************************
                          ADM_vidThreshold.cpp  -  do thresholding
                              -------------------
                          Chris MacGregor, 2005, 2007
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
 
#include "config.h"
#include "ADM_default.h"
#include "ADM_videoFilter.h"
#include "DIA_factory.h"
#include "ADM_vidThreshold.h"

// #define THRESHOLD_HISTO 1

static FILTER_PARAM thresholdParam={4,{"min", "max", "in_range_is_white", "debug"}};

SCRIPT_CREATE(threshold_script,ADMVideoThreshold,thresholdParam);

BUILD_CREATE(threshold_create,ADMVideoThreshold);

ADMVideoThreshold::ADMVideoThreshold (AVDMGenericVideoStream *in,
                                      CONFcouple *couples)
			
{
    printf ("ADMVideoThreshold ctor (%p)\n", this);
    _in = in;
    memcpy (&_info,in->getInfo(),sizeof(_info));
    _info.encoding = 1;
    _uncompressed = new ADMImage(_in->getInfo()->width, _in->getInfo()->height);
    ADM_assert (_uncompressed);
    _param = new THRESHOLD_PARAM;
    if (couples)
    {
        GET(min);
        GET(max);
        GET(in_range_is_white);
        GET(debug);
    }
    else
    {
        _param->min = 100;
        _param->max = 200;
        _param->in_range_is_white = 1;
        _param->debug = 0;
    }

    computeLookupTable (_param);
}

uint8_t ADMVideoThreshold::computeLookupTable (THRESHOLD_PARAM * param)
{
    uint8_t changed = false;

    if (param->min > param->max)
    {
        uint32_t tmp = param->min;
        param->min = param->max;
        param->max = tmp;
        param->in_range_is_white = !param->in_range_is_white;
        changed = true;
    }

    uint32_t min = param->min;
    uint32_t max = param->max;
    ADM_assert (min <= max);

    uint8_t in_range_value;
    uint8_t out_of_range_value;

    if (param->in_range_is_white)
    {
        in_range_value = 255;
        out_of_range_value = 0;
    }
    else
    {
        in_range_value = 0;
        out_of_range_value = 255;
    }

    for (uint32_t i = 0; i <= 255; i++)
    {
        if (i < min || i > max)
            lookup_table [i] = out_of_range_value;
        else
            lookup_table [i] = in_range_value;
    }

    return changed;
}

uint8_t	ADMVideoThreshold::getCoupledConf( CONFcouple **couples)
{

    ADM_assert(_param);
    *couples = new CONFcouple(thresholdParam.nb);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
    CSET(min);
    CSET(max);
    CSET(in_range_is_white);
    CSET(debug);

    return 1;
}

uint8_t ADMVideoThreshold::configure (AVDMGenericVideoStream *in)
{
    uint8_t ret = DIA_threshold (_in, this, _param);
    if (ret == 1)
    {
        computeLookupTable (_param);
        return ret;
    }
    else if (ret == 0) // 0 = cancel
    {
        computeLookupTable (_param);
        return ret;
    }
    else
    {
        ADM_assert (ret == 255); // 255 = whizzy dialog not implemented
    }

    diaElemUSlider minslide(&(_param->min), QT_TR_NOOP("Mi_nimum value to be in-range:"), 0, 255);
    diaElemUSlider maxslide(&(_param->max), QT_TR_NOOP("Ma_ximum value to be in-range:"), 0, 255);

    diaMenuEntry tInRangeIsWhite [] = {
        { 1, QT_TR_NOOP("In-range values go white, out-of-range go black"), NULL },
        { 0, QT_TR_NOOP("In-range values go black, out-of-range go white"), NULL },
    };

    diaElemMenu in_range_is_white
        (&(_param->in_range_is_white),
         QT_TR_NOOP("Output values:"),
         sizeof (tInRangeIsWhite) / sizeof (diaMenuEntry), tInRangeIsWhite);

    diaElemUInteger debug(&(_param->debug), QT_TR_NOOP("_Debugging settings (bits):"),
                          0, 0x7fffffff);
    diaElem * elems[] = { &minslide, &maxslide, &in_range_is_white, &debug };

    ret = diaFactoryRun(QT_TR_NOOP("Threshold Configuration"), sizeof (elems) / sizeof (diaElem *), elems);

    if (ret) // 0 = cancel
    {
        computeLookupTable (_param);
    }

    return ret;
}

ADMVideoThreshold::~ADMVideoThreshold()
{
    printf ("ADMVideoThreshold dtor (%p)\n", this);
    DELETE(_param);
    delete _uncompressed;	
    _uncompressed = NULL;
}

char *ADMVideoThreshold::printConf (void) 
{
    const int CONF_LEN = 100;
    static char conf[CONF_LEN];
    char * cptr = conf;
    cptr += snprintf (conf, CONF_LEN, "Threshold: In-range values (%u - %u) "
                      "go %s, others %s",
                      _param->min, _param->max,
                      _param->in_range_is_white ? "white" : "black",
                      _param->in_range_is_white ? "black" : "white");
    if (_param->debug)
        snprintf (cptr, CONF_LEN - (cptr - conf), "; debug=0x%x", _param->debug);

    return conf;
}

uint8_t ADMVideoThreshold::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          			ADMImage *data,uint32_t *flags)
{
    if (frame>= _info.nb_frames)
        return 0;

    uint32_t debug = _param->debug;

    if (debug & 1)
        printf ("in ADMVideoThreshold::getFrameNumberNoAlloc(%d, ...)\n",
                frame);

    uint32_t planesize = _info.width * _info.height;
    uint32_t size = (planesize * 3) >> 1;
    *len = size;
			
    if (!_in->getFrameNumberNoAlloc (frame, len, _uncompressed, flags))
        return 0;
    ADMImage * image = _uncompressed;

    // HERE: for speed, we do luma (Y plane) only.  However, some
    // users might want chroma, too... we should make that
    // an option or something.

    uint8_t * currp = YPLANE (image) + planesize;
    uint8_t * destp = YPLANE (data) + planesize;
    uint32_t pixremaining = planesize + 1;

    while (--pixremaining)
    {
        int32_t curr = *--currp;

        *--destp = lookup_table [curr];
    }

    // HERE: the following two lines do a luma-only-ize

    memset (UPLANE (data), 128, planesize >> 2);
    memset (VPLANE (data), 128, planesize >> 2);

    data->copyInfo (image);
    return 1;
}	                           

// This is used by the preview code for the configuration dialog.

void ADMVideoThreshold::doThreshold (ADMImage * from, ADMImage * to,
                                     ADMVideoThreshold * thresholdp,
                                     uint32_t pixelcount)
{
    uint8_t * currp = YPLANE (from) + pixelcount;
    uint8_t * destp = YPLANE (to) + pixelcount;
    uint32_t pixremaining = pixelcount + 1;
    const uint8_t * table = thresholdp->lookup_table;

    while (--pixremaining)
    {
        int32_t curr = *--currp;

        *--destp = table [curr];
    }

    // HERE: the following two lines do a luma-only-ize

    memset (UPLANE (to), 128, pixelcount >> 2);
    memset (VPLANE (to), 128, pixelcount >> 2);
}
