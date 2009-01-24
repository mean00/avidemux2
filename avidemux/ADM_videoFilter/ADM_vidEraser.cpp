/***************************************************************************
                          ADM_vidEraser.cpp  -  "Erase" arbitrary areas of
                                                        each frame
                             -------------------
                         Chris MacGregor, December 2007
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

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <iterator>

#include "ADM_assert.h"
#include "fourcc.h"
#include "avi_vars.h"

#include "ADM_toolkit/toolkit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_encoder/adm_encoder.h"


#include "DIA_factory.h"

#include "ADM_vidEraser.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_flyDialog.h" // for MenuMapping

using namespace std;

static const int MAX_PIXEL_LUMA = 255;

static FILTER_PARAM eraserParam =
{
    5,
    { "brush_mode", "brush_size", "output_color", "data_file", "debug" // 4
    }
};

// This is a hack to work around the fact that the ctor & dtor get called
// too often.  The right solution would be to arrange for the filter
// objects to be constructed and destructed only when really necessary:
// when a new instance of a filter is added to the list (by the user), it
// is constructed, and when it is removed from the list (by the user), it
// is destructed, and anything else is handled by a separate init() or
// configure() method.  This would allow the objects to maintain a
// persistent state in a more straightforward way.

ADMVideoEraser::PImap ADMVideoEraser::pimap;


SCRIPT_CREATE(eraser_script,ADMVideoEraser,eraserParam);

BUILD_CREATE(eraser_create,ADMVideoEraser);

ADMVideoEraser::ADMVideoEraser (AVDMGenericVideoStream *in, CONFcouple *couples)
{
    _in = in;
    memcpy (&_info, in->getInfo(), sizeof(_info));
    _info.encoding = 1;
    _uncompressed = new ADMImage (_in->getInfo()->width, _in->getInfo()->height);
    ADM_assert (_uncompressed);
    _param = new ERASER_PARAM;

    if (couples)
    {
        GET(brush_mode);
        GET(brush_size);
        GET(output_color);
        char * tmp;
        GET2(data_file, tmp);
        GET(debug);
    }
    else
    {
        _param->brush_mode = 1; // erase
        _param->brush_size = 1; // 3x3
        _param->output_color = 0;
        // _param->data_file = ""; // implicit
        _param->debug = 0;
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

    printf ("ADMVideoEraser ctor (%p, conf = %p), pi = %p, rc now %d\n",
            this, couples, myInfo, myInfo->refCount);
}

uint8_t	ADMVideoEraser::getCoupledConf (CONFcouple **couples)
{

    ADM_assert (_param);
    *couples = new CONFcouple (eraserParam.nb);

    // This is a hack to work around the fact that the ctor & dtor get called
    // too often.  The right solution would be to arrange for the filter
    // objects to be constructed and destructed only when really necessary:
    // when a new instance of a filter is added to the list (by the user), it
    // is constructed, and when it is removed from the list (by the user), it
    // is destructed, and anything else is handled by a separate init() or
    // configure() method.  This would allow the objects to maintain a
    // persistent state in a more straightforward way.

    printf ("ADMVideoEraser::getCoupledConf(): this = %p, couples = %p, "
            "oldConf = %p (was %p), pi = %p\n",
            this, *couples, myInfo->conf, myInfo->oldConf, myInfo);

    if (myInfo->oldConf)
        pimap.erase (myInfo->oldConf);
    myInfo->oldConf = myInfo->conf;
    myInfo->conf = *couples;
    pimap [myInfo->conf] = myInfo;
    if (myInfo->oldConf == 0)
        pimap.erase (0);

    CSET(brush_mode);
    CSET(brush_size);
    CSET(output_color);
    (*couples)->setCouple("data_file", _param->data_file.c_str());
    CSET(debug);

    return 1;
}

uint8_t ADMVideoEraser::configure (AVDMGenericVideoStream *in)
{
    diaMenuEntry tBrushMode [] = {
        { 1, QT_TR_NOOP("Erase"), NULL },
        { 0, QT_TR_NOOP("Un-Erase"), NULL },
    };

    diaMenuEntry tBrushSize [] = {
        { 0, QT_TR_NOOP("1x1"), NULL },
        { 1, QT_TR_NOOP("3x3"), NULL },
        { 2, QT_TR_NOOP("5x5"), NULL },
        { 3, QT_TR_NOOP("7x7"), NULL },
        { 4, QT_TR_NOOP("9x9"), NULL },
        { 5, QT_TR_NOOP("11x11"), NULL },
        { 7, QT_TR_NOOP("15x15"), NULL },
        { 10, QT_TR_NOOP("21x21"), NULL },
    };

    diaElemMenu brush_mode
        (&(_param->brush_mode), QT_TR_NOOP("Brush _Mode:"),
         sizeof (tBrushMode) / sizeof (diaMenuEntry), tBrushMode);

    diaElemMenu brush_size
        (&(_param->brush_size), QT_TR_NOOP("Brush _Size:"),
         sizeof (tBrushSize) / sizeof (diaMenuEntry), tBrushSize);

    MenuMapping menu_mapping [] = {
        { "brushModeMenu", my_offsetof (ERASER_PARAM, brush_mode),
          sizeof (tBrushMode) / sizeof (diaMenuEntry), tBrushMode },
        { "brushSizeMenu", my_offsetof (ERASER_PARAM, brush_size),
          sizeof (tBrushSize) / sizeof (diaMenuEntry), tBrushSize },
    };

    if (myInfo->masks.empty() || myInfo->mask_data_invalid)
        readDataFile (_info.width);

    // printf ("ADM_vidEraser: _param = %p\n", _param);
    uint8_t ret = DIA_eraser (_in, this, _param, menu_mapping,
                              sizeof (menu_mapping) / sizeof (MenuMapping));
    if (ret == 1)
    {
        writeDataFile();
        return ret;
    }
    else if (ret == 0) // 0 = cancel
    {
        myInfo->mask_data_invalid = true;
        return ret;
    }
    else
    {
        ADM_assert (ret == 255); // 255 = whizzy dialog not implemented
    }

    diaElemUSlider output_color
        (&(_param->output_color),
         QT_TR_NOOP("Output \"_Color\" for all masked pixels:"), 0, 255);

    char * file = ADM_strdup (_param->data_file.c_str());

    diaElemFile data_file
        (0, &file,
         QT_TR_NOOP("Eraser _Data File:"), 0, QT_TR_NOOP("Select data file"));

    // TODO: The configuration of the masks is not implemented!!!

    diaElemUInteger debug
        (&(_param->debug), QT_TR_NOOP("_Debugging settings (bits):"),
         0, 0x7fffffff);

    diaElem * elems[] = { &brush_mode, &brush_size, &output_color,
                          &data_file, &debug };

    ret = diaFactoryRun (QT_TR_NOOP("Eraser Configuration"),
                         sizeof (elems) / sizeof (diaElem *), elems);
    if (ret) // 0 = cancel
    {
        writeDataFile();
        myInfo->mask_data_invalid = true;
    }

    _param->data_file = file;
    delete[] file;

    return ret;
}

ADMVideoEraser::~ADMVideoEraser()
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
    printf ("ADMVideoEraser dtor (%p), conf = %p, pi = %p, rc now %d\n",
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

    DELETE (_param);
    delete _uncompressed;
    _uncompressed = NULL;
}

char * ADMVideoEraser::printConf ()
{
    const int CONF_LEN = 1024;
    static char conf[CONF_LEN];

    const char * data_file = _param->data_file.c_str();
    if (!data_file || !*data_file)
        data_file = "**** no file selected ****";

    char * cptr = conf;
    cptr += snprintf (conf, CONF_LEN, "Eraser: erase to %d, data in %s",
                      _param->output_color, data_file);

    if (_param->debug)
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          ", debug = 0x%x", _param->debug);
    return conf;
}

void ADMVideoEraser::writeDataFile () const
{
    const char * filename = _param->data_file.c_str();
    ofstream outputStream (filename);
    if (!outputStream)
    {
        perror (filename);
        fprintf (stderr, "******** FAILED to write eraser data to %s (%d)\n",
                 filename, errno);
        return;
    }

    Eraser::MaskVec & masks = myInfo->masks;

    outputStream << "# avidemux Eraser video filter data\n"
                 << "version: 1"
                 << "\ndimensions: " << _info.width << " " << _info.height
                 << "\nmaskcount: " << masks.size()
                 << "\n";

    for (Eraser::MaskVec::const_iterator maskit = masks.begin();
         maskit != masks.end();
         ++maskit)
    {
        const Eraser::Mask & mask = *maskit;
        const Eraser::LineVec & lines = mask.lines;
        outputStream << "\nmask: " << mask.first_frame << " "
                     << mask.last_frame << " " << lines.size() << "\n";

        for (Eraser::LineVec::const_iterator lineit = lines.begin();
             lineit != lines.end();
             ++lineit)
        {
            outputStream << lineit->x << " " << lineit->y << " "
                         << lineit->count << "\n";
        }
    }

    outputStream << "\nend\n";
}

uint8_t ADMVideoEraser::readDataFile (uint32_t width)
{
    Eraser::MaskVec & masks = myInfo->masks;

//    uint32_t & mask_w = myInfo->mask_w;
//    uint32_t & mask_h = myInfo->mask_h;

    const char * filename = _param->data_file.c_str();
    if (filename[0] == '\0')
    {
        fprintf (stderr, "Eraser: no input file selected!\n");
        return 0;
    }

//    mask_w = 0;
//    mask_h = 0;

    ifstream inputStream (filename);
    if (!inputStream)
    {
        fprintf (stderr, "Eraser: can't open input file %s, "
                 "but it apparently does exist...(%d)\n",
                 filename, errno);
        return 0;
    }

    masks.clear();
    string buffer;

    while (inputStream)
    {
        inputStream >> buffer;
        if (buffer [0] == '#')
        {
            // toss rest of line
            inputStream.ignore (1000000,'\n');
        }
        else if (buffer == "mask:")
        {
            uint32_t first_frame, last_frame, linecount;
            inputStream >> first_frame >> last_frame >> linecount;
            masks.push_back (Eraser::Mask (first_frame, last_frame));
            Eraser::MaskVec::iterator maskit = masks.end() - 1;
            Eraser::LineVec & lines = maskit->lines;
            lines.reserve (linecount);
            while (linecount--)
            {
                uint16_t x, y, count;
                inputStream >> x >> y >> count;
                if (count == 0)
                {
                    fprintf (stderr, "%s: bad count (x = %d, y = %d)\n",
                             filename, x, y);
                    return 0;
                }
                lines.push_back (Eraser::Line (x, y, count));
            }
            printf ("mask %d: %d lines\n", masks.size() - 1, lines.size());
        }
        else if (buffer == "maskcount:")
        {
            uint32_t count;
            inputStream >> count;
            masks.reserve (count);
        }
        else if (buffer == "version:")
        {
            int version;
            inputStream >> version;
            if (version != 1)
            {
                fprintf (stderr, "%s: unsupported version: %d\n",
                         filename, version);
                return 0;
            }
        }
        else if (buffer == "dimensions:")
        {
            int dummy;
            inputStream >> dummy; // width
            inputStream >> dummy; // height
        }
        else if (buffer != "end")
        {
            fprintf (stderr, "%s: unrecognized gunk: \"%s\"\n",
                     filename, buffer.c_str());
            return 0;
        }
    }

    myInfo->mask_data_invalid = false;
    return 1;
}

//============================================================================

uint8_t ADMVideoEraser::getFrameNumberNoAlloc (uint32_t frame, uint32_t *len,
                                               ADMImage *data, uint32_t *flags)
{
    if (frame >= _info.nb_frames)
        return 0;

    if (_param->debug & 1)
        printf ("in ADMVideoEraser::getFrameNumberNoAlloc(%d, ...)\n", frame);

    if (!_in->getFrameNumberNoAlloc (frame, len, _uncompressed, flags))
        return 0;

    if (myInfo->masks.empty() || myInfo->mask_data_invalid)
        readDataFile (_info.width);

    uint32_t planesize = _info.width * _info.height;
    uint32_t size = (planesize * 3) >> 1;
    *len = size;

    uint32_t real_frame = frame + _info.orgFrame;
    uint8_t ret = doEraser (_uncompressed, data, _in, real_frame,
                            this, _param, _info.width, _info.height);
    return ret;
}

uint8_t
ADMVideoEraser::doEraser (ADMImage * image, ADMImage * data,
                          AVDMGenericVideoStream * in,
                          uint32_t real_frame,
                          ADMVideoEraser * eraser, ERASER_PARAM * param,
                          uint32_t width, uint32_t height)
{
    PersistentInfo * myInfo = eraser->myInfo;
    uint32_t debug = param->debug;

    Eraser::MaskVec & masks = myInfo->masks;
    uint32_t planesize = width * height;
    uint8_t * outPixels = YPLANE (data);

    memcpy (outPixels, YPLANE (image), planesize);

    // HERE: for better performance, especially with more than a small number
    // of masks, figure out how to use lower_bound() or some other binary
    // search type of thing (write our own if necessary to handle comparing an
    // int to an Eraser::Mask) to find the mask whose range includes
    // real_frame, rather than linear searching on every frame.

    for (Eraser::MaskVec::const_iterator maskit = masks.begin();
         maskit != masks.end();
         ++maskit)
    {
        const Eraser::Mask & mask = *maskit;

        if (real_frame < mask.first_frame)
            break; // they're sorted, so no more will match

        if (real_frame > mask.last_frame)
            continue;

        const Eraser::LineVec & lines = mask.lines;
        for (Eraser::LineVec::const_iterator lineit = lines.begin();
             lineit != lines.end();
             ++lineit)
        {
            memset (outPixels + (lineit->y * width) + lineit->x,
                    param->output_color, lineit->count);
        }

        break; // only one can match any given frame number, since they can't
               // overlap.
    }

    // HERE: the following two lines do a luma-only-ize

    memset (UPLANE (data), 128, planesize >> 2);
    memset (VPLANE (data), 128, planesize >> 2);

    data->copyInfo (image);

    return 1;
}	                           
