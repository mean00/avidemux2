/***************************************************************************
                          ADM_vidComputeAverage.cpp  -  compute average of
                                                     all frames (so it can
                                                     later be subtracted to
                                                     remove static
                                                     background details)
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
 
#include "ADM_default.h"
#include "ADM_videoFilter.h"

#include "DIA_factory.h"
#include "ADM_vidComputeAverage.h"

static FILTER_PARAM computeAverageParam={5,{"start_frame",
                                            "end_frame",
                                            "output_file",
                                            "bias",
                                            "display_mode"}};

// This is a hack to work around the fact that the ctor & dtor get called
// too often.  The right solution would be to arrange for the filter
// objects to be constructed and destructed only when really necessary:
// when a new instance of a filter is added to the list (by the user), it
// is constructed, and when it is removed from the list (by the user), it
// is destructed, and anything else is handled by a separate init() or
// configure() method.  This would allow the objects to maintain a
// persistent state in a more straightforward way.

ADMVideoComputeAverage::PImap ADMVideoComputeAverage::pimap;


SCRIPT_CREATE(computeaverage_script,ADMVideoComputeAverage,computeAverageParam);

BUILD_CREATE(computeaverage_create,ADMVideoComputeAverage);

ADMVideoComputeAverage::ADMVideoComputeAverage(AVDMGenericVideoStream *in,CONFcouple *couples)
			
{
    _in = in;
    memcpy (&_info, in->getInfo(), sizeof(_info));
    _info.encoding = 1;	
    _uncompressed = new ADMImage (_in->getInfo()->width, _in->getInfo()->height);
    ADM_assert(_uncompressed);
    _param = new COMPUTEAVERAGE_PARAM;

    if (couples)
    {
        GET(start_frame);
        GET(end_frame);
        char * tmp;
        GET2(output_file, tmp);
        GET(bias);
        GET(display_mode);
    }
    else
    {
        _param->start_frame = 0;
        _param->end_frame = -1;
        _param->output_file = ADM_strdup ("");
        _param->bias = 128;
        _param->display_mode = DISPLAYMODE_FRAME_MINUS_AVERAGE;
    }

    // This is a hack to work around the fact that the ctor & dtor get called
    // too often.  The right solution would be to arrange for the filter
    // objects to be constructed and destructed only when really necessary:
    // when a new instance of a filter is added to the list (by the user), it
    // is constructed, and when it is removed from the list (by the user), it
    // is destructed, and anything else is handled by a separate init() or
    // configure() method.  This would allow the objects to maintain a
    // persistent state in a more straightforward way.

    // This explicit check wouldn't be necessary if there was an easier way of
    // ensuring that we got a 0 pointer when a new map entry was auto-consed
    // up...

    if (pimap.count (couples) == 0)
    {
        myInfo = new PersistentInfo;
        myInfo->conf = couples;
        pimap [couples] = myInfo;
    }
    else
    {
        myInfo = pimap [couples];
        if (couples)
        {
            if (myInfo->oldConf == couples)
            {
                pimap.erase (myInfo->conf);
                myInfo->conf = myInfo->oldConf;
                myInfo->oldConf = 0;
            }
            else
            {
                ADM_assert (myInfo->conf == couples);

                if (myInfo->oldConf)
                {
                    pimap.erase (myInfo->oldConf);
                    myInfo->oldConf = 0;
                }
            }
        }
    }

    myInfo->refCount++;

    printf ("ADMVideoComputeAverage ctor (%p, conf = %p), pi = %p, rc now %d\n",
            this, couples, myInfo, myInfo->refCount);
}

uint8_t	ADMVideoComputeAverage::getCoupledConf (CONFcouple **couples)
{

    ADM_assert(_param);
    *couples=new CONFcouple(computeAverageParam.nb);

    // This is a hack to work around the fact that the ctor & dtor get called
    // too often.  The right solution would be to arrange for the filter
    // objects to be constructed and destructed only when really necessary:
    // when a new instance of a filter is added to the list (by the user), it
    // is constructed, and when it is removed from the list (by the user), it
    // is destructed, and anything else is handled by a separate init() or
    // configure() method.  This would allow the objects to maintain a
    // persistent state in a more straightforward way.

    printf("ADMVideoComputeAverage::getCoupledConf(): this = %p, couples = %p, "
           "oldConf = %p (was %p), pi = %p\n",
           this, *couples, myInfo->conf, myInfo->oldConf, myInfo);

    if (myInfo->oldConf)
        pimap.erase (myInfo->oldConf);
    myInfo->oldConf = myInfo->conf;
    myInfo->conf = *couples;
    pimap [myInfo->conf] = myInfo;
    if (myInfo->oldConf == 0)
        pimap.erase (0);

#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
    CSET(start_frame);
    CSET(end_frame);
    CSET(output_file);
    CSET(bias);
    CSET(display_mode);

    return 1;

}

uint8_t ADMVideoComputeAverage::configure(AVDMGenericVideoStream *in)
{
    UNUSED_ARG(in);

    diaElemInteger start_frame
        (&(_param->start_frame),
         QT_TR_NOOP("_Start Frame (first frame # to include in average):"),
         0, 0x7fffffff);
    diaElemInteger end_frame
        (&(_param->end_frame),
         QT_TR_NOOP("_End Frame (last frame # to include), -1 = last:"),
         -1000000, 0x7fffffff);
    diaElemFile output_file
        (1, const_cast<char **>(&(_param->output_file)),
         QT_TR_NOOP("_Output File:"), NULL, QT_TR_NOOP("Select output file"));
    diaElemSlider bias
        (&(_param->bias),
         QT_TR_NOOP("_Bias (only for display; use 0 for "
           "average, 128 for frame minus average):"), -256, +256);
    diaMenuEntry tDisplayMode [] = {
        { DISPLAYMODE_FRAME_MINUS_AVERAGE,
          QT_TR_NOOP("Current frame minus average so far"), NULL },
        { DISPLAYMODE_AVERAGE, QT_TR_NOOP("Average so far"), NULL },
        { DISPLAYMODE_BLANK,
          QT_TR_NOOP("Display nothing (fast for batch processing)"), NULL },
    };
    diaElemMenu display_mode
        (&(_param->display_mode),
         QT_TR_NOOP("Display _Mode:"),
         sizeof (tDisplayMode) / sizeof (diaMenuEntry), tDisplayMode);
    diaElem * elems[] = { &start_frame, &end_frame, &output_file,
                          &bias, &display_mode };

    uint8_t ret = diaFactoryRun(QT_TR_NOOP("ComputeAverage"), sizeof (elems) / sizeof (diaElem *), elems);
    return ret;
	
}

ADMVideoComputeAverage::~ADMVideoComputeAverage()
{
    // This is a hack to work around the fact that the ctor & dtor get called
    // too often.  The right solution would be to arrange for the filter
    // objects to be constructed and destructed only when really necessary:
    // when a new instance of a filter is added to the list (by the user), it
    // is constructed, and when it is removed from the list (by the user), it
    // is destructed, and anything else is handled by a separate init() or
    // configure() method.  This would allow the objects to maintain a
    // persistent state in a more straightforward way.

    myInfo->refCount--;
    printf ("ADMVideoComputeAverage dtor (%p), conf = %p, pi = %p, rc now %d\n",
            this, myInfo->conf, myInfo, myInfo->refCount);

    if (myInfo->oldConf)
    {
        pimap.erase (myInfo->oldConf);
        myInfo->oldConf = 0;
    }

    if (myInfo->refCount < 1)
    {
        pimap.erase (myInfo->conf);
        delete myInfo;
    }

    DELETE(_param);
    delete  _uncompressed;	
    _uncompressed=NULL;
}

char * ADMVideoComputeAverage::printConf (void) 
{
    const int CONF_LEN = 512;
    static char conf[CONF_LEN];

    const char * display = "oops";
    if (_param->display_mode == DISPLAYMODE_AVERAGE)
        display = "average so far";
    else if (_param->display_mode == DISPLAYMODE_FRAME_MINUS_AVERAGE)
        display = "current frame minus average so far";
    else if (_param->display_mode == DISPLAYMODE_BLANK)
        display = "nothing (fast)";

    snprintf(conf, CONF_LEN, "ComputeAverage: Average of frames %d - %d, "
             "output to %s, display %s %+d",
             _param->start_frame, _param->end_frame, _param->output_file,
             display, _param->bias);
    return conf;
	
}

static inline
uint8_t saturate_pixel (uint32_t pixel)
{
    if (pixel & 0xffffff00)
    {
        if (int32_t (pixel) < 0)
            return 0;
        else // if (pixel > 255)
            return 255;
    }

    return pixel;
}

uint8_t ADMVideoComputeAverage::getFrameNumberNoAlloc (uint32_t frame, uint32_t *len,
                                                       ADMImage *data, uint32_t *flags)
{
    if (frame >= _info.nb_frames)
        return 0;

    int debug = 0;

    int curr_frame = frame + _info.orgFrame;
    int num_frames = _info.nb_frames + _info.orgFrame;

    int planesize = _info.width * _info.height;
    uint32_t size = (planesize * 3) >> 1;
    *len = size;
			
    if (!_in->getFrameNumberNoAlloc (frame, len, _uncompressed, flags))
        return 0;
    ADMImage * image = _uncompressed;

    uint32_t start_frame = _param->start_frame;
    uint32_t end_frame = _param->end_frame;
    if (int32_t (end_frame) < 0)
        end_frame = int32_t (num_frames) + int32_t (end_frame); // 0-based, thus -1 = last frame

    if (!myInfo->sums
        || myInfo->width != _info.width || myInfo->height != _info.height
        || myInfo->start_frame != start_frame
        || myInfo->end_frame != end_frame)
    {
        printf ("ADMVideoComputeAverage: resetting; was %p %ux%u, %u..%u\n",
                myInfo->sums, myInfo->width, myInfo->height,
                myInfo->start_frame, myInfo->end_frame);

        if (!myInfo->sums
            || myInfo->width != _info.width || myInfo->height != _info.height)
        {
            myInfo->width = _info.width;
            myInfo->height = _info.height;
            delete[] myInfo->sums;
            myInfo->sums = new uint32_t [planesize];
        }

        uint32_t pixremaining = planesize + 1;
        uint32_t * sums = myInfo->sums + planesize;
        while (--pixremaining)
        {
            *--sums = 0;
        }

        myInfo->frame_count = 0;
        myInfo->most_recent_frame = -1;
        myInfo->start_frame = start_frame;
        myInfo->end_frame = end_frame;

        printf ("ADMVideoComputeAverage: reset average; now %p %ux%u, "
                "%u..%u\n",
                myInfo->sums, myInfo->width, myInfo->height,
                myInfo->start_frame, myInfo->end_frame);
    }
			
    // HERE: for speed, we do luma (Y plane) only.  However, some
    // users might want chroma, too... we should make that
    // an option or something.

    uint8_t * currp = YPLANE (image) + planesize;
    uint8_t * destp = YPLANE (data) + planesize;
    uint32_t * sums = myInfo->sums + planesize;
    uint32_t pixremaining = planesize + 1;
    int32_t bias = _param->bias;

    if (curr_frame >= start_frame && curr_frame <= end_frame
        && curr_frame != myInfo->most_recent_frame)
    {
        uint32_t frame_count = ++(myInfo->frame_count);
        uint32_t half_frame_count = frame_count / 2;
        if (debug & 2)
            printf ("Including frame %u of %u, now have %u (%u - %u)\n",
                    curr_frame, num_frames, frame_count, start_frame, end_frame);

        if (_param->display_mode == DISPLAYMODE_AVERAGE)
        {
            while (--pixremaining)
            {
                uint8_t curr = *--currp;
                uint32_t sum = (*--sums += curr);

                uint32_t avg = (sum + half_frame_count) / frame_count;
                *--destp = saturate_pixel (avg + bias);
            }
        }
        else if (_param->display_mode == DISPLAYMODE_FRAME_MINUS_AVERAGE)
        {
            while (--pixremaining)
            {
                uint8_t curr = *--currp;
                uint32_t sum = (*--sums += curr);

                uint32_t avg = (sum + half_frame_count) / frame_count;
                *--destp = saturate_pixel (curr + bias - avg);
            }
        }
        else // if (_param->display_mode == DISPLAYMODE_BLANK)
        {
            while (--pixremaining)
            {
                *--sums += *--currp;
            }
        }

        myInfo->most_recent_frame = curr_frame;

        // If we just did the last frame of the specified range, then we need
        // to write the output file.

        if (curr_frame == end_frame
            && frame_count == end_frame - start_frame + 1)
            write_output_file();
    }
    else // frame out of specified range for collecting average
    {
        uint32_t frame_count = myInfo->frame_count;
        uint32_t half_frame_count = frame_count / 2;
        if (debug & 2)
            printf ("Using %u frame(s) (%u - %u) worth of average on frame %u of %u\n",
                    frame_count, start_frame, end_frame, curr_frame, num_frames);

        if (frame_count == 0)
        {
            // dividing by zero is not a good idea.  Just punt.
            return 0;
        }
        else if (_param->display_mode == DISPLAYMODE_AVERAGE)
        {
            while (--pixremaining)
            {
                uint32_t sum = *--sums;
                uint32_t avg = (sum + half_frame_count) / frame_count;
                *--destp = saturate_pixel (avg + bias);
            }
        }
        else if (_param->display_mode == DISPLAYMODE_FRAME_MINUS_AVERAGE)
        {
            while (--pixremaining)
            {
                uint8_t curr = *--currp;
                uint32_t sum = *--sums;

                uint32_t avg = (sum + half_frame_count) / frame_count;
                *--destp = saturate_pixel (curr + bias - avg);
            }
        }
        else if (curr_frame > end_frame)
        {
            // nothing more to do
            return 0;
        }
    }

    if (_param->display_mode != DISPLAYMODE_BLANK)
    {
        // HERE: the following two lines do a luma-only-ize

        memset (UPLANE (data), 128, planesize >> 2);
        memset (VPLANE (data), 128, planesize >> 2);
    }

    data->copyInfo(image);
    return 1;
}

void ADMVideoComputeAverage::write_output_file () const
{
    const char * output_file = _param->output_file;
    if (!output_file || !*output_file)
    {
        fprintf (stderr, "ADMVideoComputeAverage: Wanted to write output "
                 "file, but no file has been specified!!\n");
        return;
    }

    FILE * fp = fopen (output_file, "wb");
    if (!fp)
    {
        perror (output_file);
        return;
    }

    uint32_t width = myInfo->width;
    uint32_t height = myInfo->height;

    int pixelcount = width * height;
    uint32_t * sums = myInfo->sums;
    uint32_t pixremaining = pixelcount + 1;

    FileHeader header;
    memcpy (header.magic, "DGCMimgF", sizeof (header.magic));
    header.width = width;
    header.height = height;

    int nwritten = fwrite (&header, sizeof (FileHeader), 1, fp);
    if (nwritten != 1)
    {
        fprintf (stderr, "Failed to write file header to %s\n", output_file);
        perror (output_file);
        fclose (fp);
        unlink (output_file);
        return;
    }

    uint32_t frame_count = myInfo->frame_count;

    printf ("frame count = %u\n", frame_count);

    float minavg = 1000;
    float maxavg = -1000;
    while (--pixremaining)
    {
        uint32_t sum = *sums++;
        float avg = float (sum) / frame_count;

        if (avg < minavg)
            minavg = avg;
        if (avg > maxavg)
            maxavg = avg;

        nwritten = fwrite (&avg, sizeof (float), 1, fp);
        if (nwritten != 1)
        {
            fprintf (stderr, "Failed to write pixel %u to %s\n",
                     pixelcount - pixremaining + 1, output_file);
            fflush (stdout);
            perror (output_file);
            fclose (fp);
            unlink (output_file);
            return;
        }
    }

    fprintf (stderr, "Successfully wrote %ux%u = %u floating point pixel "
             "averages (from %.6f to %.6f) of %u frames to %s\n",
             width, height, pixelcount, minavg, maxavg, frame_count,
             output_file);

    fclose (fp);
}
