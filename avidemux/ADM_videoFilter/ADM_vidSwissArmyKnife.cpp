/***************************************************************************
                          ADM_vidSwissArmyKnife.cpp  -  Perform one of many
                                                        possible operations
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

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <iterator>

#include "fourcc.h"
#include "avi_vars.h"

#include "ADM_toolkit/toolkit.hxx"
#include "ADM_videoFilter.h"

#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_encoder/adm_encoder.h"

#include "ADM_filter/video_filters.h"

#include "DIA_factory.h"

#include "ADM_vidSwissArmyKnife.h"
#include "ADM_vidParticle.h" // for ImageTool
#include "ADM_vidComputeAverage.h" // for ADMVideoComputeAverage::FileHeader
#include "ADM_userInterfaces/ADM_commonUI/DIA_flyDialog.h" // for MenuMapping

static const int MAX_PIXEL_LUMA = 255;

static FILTER_PARAM swissArmyKnifeParam =
{
    16,
    { "tool", "input_type", "input_file", "load_bias",              // 4
      "load_multiplier", "input_constant", "memory_constant_alpha", // + 3 = 7
      "lookahead_n_frames", "init_start_frame", "init_end_frame",   // + 3 = 10
      "init_by_rolling", "bias", "result_bias",                     // + 3 = 13
      "result_multiplier", "histogram_frame_interval", "debug"      // + 3 = 16
    }
};

ADMVideoSwissArmyKnife::ToolMap ADMVideoSwissArmyKnife::tool_map [] =
{
    { TOOL_A,         "A",        "%s",        "%s"     },
    { TOOL_P,         "P",        "P",         "P"      },
    { TOOL_P_MINUS_A, "P-A",      "P-%s",      "P - %s" },
    { TOOL_A_MINUS_P, "A-P",      "%s-P",      "%s - P" },
    { TOOL_P_PLUS_A,  "P+A",      "P+%s",      "P + %s" },
    { TOOL_P_TIMES_A, "P*A",      "P*%s",      "P * %s" },
    { TOOL_P_DIVBY_A, "P/A",      "P/%s",      "P / %s" },
    { TOOL_A_DIVBY_P, "A/P",      "%s/P",      "%s / P" },
    { TOOL_MIN_P_A,   "min(P,A)", "min(P,%s)", "min (P, %s)" },
    { TOOL_MAX_P_A,   "max(P,A)", "max(P,%s)", "max (P, %s)" },

    { TOOL_INVALID, 0, 0, 0 }
};

// This is a hack to work around the fact that the ctor & dtor get called
// too often.  The right solution would be to arrange for the filter
// objects to be constructed and destructed only when really necessary:
// when a new instance of a filter is added to the list (by the user), it
// is constructed, and when it is removed from the list (by the user), it
// is destructed, and anything else is handled by a separate init() or
// configure() method.  This would allow the objects to maintain a
// persistent state in a more straightforward way.

ADMVideoSwissArmyKnife::PImap ADMVideoSwissArmyKnife::pimap;


SCRIPT_CREATE(swissarmyknife_script,ADMVideoSwissArmyKnife,swissArmyKnifeParam);

BUILD_CREATE(swissarmyknife_create,ADMVideoSwissArmyKnife);

ADMVideoSwissArmyKnife::ADMVideoSwissArmyKnife (AVDMGenericVideoStream *in, CONFcouple *couples)
{
    _in = in;
    memcpy (&_info, in->getInfo(), sizeof(_info));
    _info.encoding = 1;
    _uncompressed = new ADMImage (_in->getInfo()->width, _in->getInfo()->height);
    ADM_assert (_uncompressed);
    _param = new SWISSARMYKNIFE_PARAM;

    if (couples)
    {
        GET(tool);
        GET(input_type);

        char* tmp;
        GET2(input_file, tmp);
        GET(load_bias);
        GET(load_multiplier);

        GET(input_constant);

        GET(memory_constant_alpha);
        GET(lookahead_n_frames);
        GET(init_start_frame);
        GET(init_end_frame);
        GET(init_by_rolling);

        GET(bias);
        GET(result_bias);
        GET(result_multiplier);
        GET(histogram_frame_interval);
        GET(debug);

        _param->enable_preview
            = (_param->input_type != INPUT_ROLLING_AVERAGE);
    }
    else
    {
        _param->tool = TOOL_P_MINUS_A;
        _param->input_type = INPUT_ROLLING_AVERAGE;

        // _param->input_file = ""; // implicit
        _param->load_bias = 0.0;
        _param->load_multiplier = 1.0;

        _param->input_constant = 0;

        _param->memory_constant_alpha = 1.0 / _param->init_end_frame;
        _param->lookahead_n_frames = 0;
        _param->init_start_frame = 0;
        _param->init_end_frame = 99;
        _param->init_by_rolling = false;

        _param->bias = 128;
        _param->result_bias = 0.0;
        _param->result_multiplier = 1.0;
        _param->histogram_frame_interval = 0;
        _param->debug = 0;

        _param->enable_preview = false;
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

    printf ("ADMVideoSwissArmyKnife ctor (%p, conf = %p), pi = %p, rc now %d\n",
            this, couples, myInfo, myInfo->refCount);
}

uint8_t	ADMVideoSwissArmyKnife::getCoupledConf (CONFcouple **couples)
{

    ADM_assert (_param);
    *couples = new CONFcouple (swissArmyKnifeParam.nb);

    // This is a hack to work around the fact that the ctor & dtor get called
    // too often.  The right solution would be to arrange for the filter
    // objects to be constructed and destructed only when really necessary:
    // when a new instance of a filter is added to the list (by the user), it
    // is constructed, and when it is removed from the list (by the user), it
    // is destructed, and anything else is handled by a separate init() or
    // configure() method.  This would allow the objects to maintain a
    // persistent state in a more straightforward way.

    printf("ADMVideoSwissArmyKnife::getCoupledConf(): this = %p, couples = %p, "
           "oldConf = %p (was %p), pi = %p\n",
           this, *couples, myInfo->conf, myInfo->oldConf, myInfo);

    if (myInfo->oldConf)
        pimap.erase (myInfo->oldConf);
    myInfo->oldConf = myInfo->conf;
    myInfo->conf = *couples;
    pimap [myInfo->conf] = myInfo;
    if (myInfo->oldConf == 0)
        pimap.erase (0);

    CSET(tool);
    CSET(input_type);

    (*couples)->setCouple("input_file", _param->input_file.c_str());
    CSET(load_bias);
    CSET(load_multiplier);

    CSET(input_constant);

    CSET(memory_constant_alpha);
    CSET(lookahead_n_frames);
    CSET(init_start_frame);
    CSET(init_end_frame);
    CSET(init_by_rolling);

    CSET(bias);
    CSET(result_bias);
    CSET(result_multiplier);
    CSET(histogram_frame_interval);
    CSET(debug);

    return 1;
}

uint8_t ADMVideoSwissArmyKnife::configure (AVDMGenericVideoStream *in)
{
    diaMenuEntry tTool [] = {
        { TOOL_A,          QT_TR_NOOP("P' = A"), NULL },
        { TOOL_P,          QT_TR_NOOP("P' = P"), NULL },
        { TOOL_P_MINUS_A,  QT_TR_NOOP("P' = P - A"), NULL },
        { TOOL_A_MINUS_P,  QT_TR_NOOP("P' = A - P"), NULL },
        { TOOL_P_PLUS_A,   QT_TR_NOOP("P' = P + A"), NULL },
        { TOOL_P_TIMES_A,  QT_TR_NOOP("P' = P * A"), NULL },
        { TOOL_P_DIVBY_A,  QT_TR_NOOP("P' = P / A"), NULL },
        { TOOL_A_DIVBY_P,  QT_TR_NOOP("P' = A / P"), NULL },
        { TOOL_MIN_P_A,    QT_TR_NOOP("P' = min (P, A)"), NULL },
        { TOOL_MAX_P_A,    QT_TR_NOOP("P' = max (P, A)"), NULL }
    };

    diaMenuEntry tInputType [] = {
        { INPUT_CUSTOM_CONVOLUTION,
          QT_TR_NOOP("A = convolve(P); Load convolution kernel from file"), NULL },
        { INPUT_FILE_IMAGE_FLOAT,
          QT_TR_NOOP("A = pixel from image file as float; Load image from file"), NULL },
        { INPUT_FILE_IMAGE_INTEGER,
          QT_TR_NOOP("A = pixel from image file as integer; Load image from file"), NULL },
        { INPUT_CONSTANT_VALUE,
          QT_TR_NOOP("A = floating point constant value"), NULL },
        { INPUT_ROLLING_AVERAGE,
          QT_TR_NOOP("A = rolling average of pixel: A = A*(1-alpha)+(P*alpha)"), NULL },
    };

    diaElemMenu tool
        (&(_param->tool),
         QT_TR_NOOP("Select _Operation on each pixel P and input A:"),
         sizeof (tTool) / sizeof (diaMenuEntry), tTool);

    diaElemMenu input_type
        (&(_param->input_type),
         QT_TR_NOOP("Input _Type:"),
         sizeof (tInputType) / sizeof (diaMenuEntry), tInputType);

    MenuMapping menu_mapping [] = {
        { "operationMenu", my_offsetof (SWISSARMYKNIFE_PARAM, tool),
          sizeof (tTool) / sizeof (diaMenuEntry), tTool },
        { "inputTypeMenu", my_offsetof (SWISSARMYKNIFE_PARAM, input_type),
          sizeof (tInputType) / sizeof (diaMenuEntry), tInputType },
    };

    // printf ("ADM_vidSwissArmyKnife: _param = %p\n", _param);
    uint8_t ret = DIA_SwissArmyKnife (_in, this, _param, menu_mapping,
                                      sizeof (menu_mapping)
                                      / sizeof (MenuMapping));
    if (ret == 1)
    {
        return ret;
    }
    else if (ret == 0) // 0 = cancel
    {
        return ret;
    }
    else
    {
        ADM_assert (ret == 255); // 255 = whizzy dialog not implemented
    }

    char * file = ADM_strdup (_param->input_file.c_str());

    diaElemFile input_file
        (0, &file,
         QT_TR_NOOP("Input _File (image or convolution kernel):"), NULL, QT_TR_NOOP("Select file"));
    diaElemFloat load_bias
        (&(_param->load_bias),
         QT_TR_NOOP("_Load Bias (added to each pixel\n"
           "in file image when loaded):"),
         -99999, +99999); // arbitrary!
    diaElemFloat load_multiplier
        (&(_param->load_multiplier),
         QT_TR_NOOP("Load _Multiplier (each pixel in\n"
           "file image mult. by this when loaded):"),
         -99999, +99999); // arbitrary!

    diaElemFloat input_constant
        (&(_param->input_constant),
         QT_TR_NOOP("Input _Constant:"), -99999, +99999); // arbitrary!

    diaElemFloat memory_constant_alpha
        (&(_param->memory_constant_alpha),
         QT_TR_NOOP("Memory constant _alpha\n"
           "(where A = (1-alpha)*A + alpha*(curr_frame + lookahead)):"),
         0, 0x7fffffff);
    diaElemUInteger lookahead_n_frames
        (&(_param->lookahead_n_frames),
         QT_TR_NOOP("Look ahead _N frames:"),
         0, 0x7fffffff);
    diaElemUInteger init_start_frame
        (&(_param->init_start_frame),
         QT_TR_NOOP("Init _Start Frame (first frame # to use for head start):"),
         0, 0x7fffffff);
    diaElemUInteger init_end_frame
        (&(_param->init_end_frame),
         QT_TR_NOOP("Init _End Frame (last frame # to use for head start):"),
         0, 0x7fffffff);
    diaElemToggle init_by_rolling
        (&(_param->init_by_rolling),
         QT_TR_NOOP("Init By _Rolling (compute head start using a "
           "rolling average rather than a straight average)"));

    diaElemSlider bias
        (&(_param->bias),
         QT_TR_NOOP("_Bias (will be added to result):"), -256, +256);
    diaElemFloat result_bias
        (&(_param->result_bias),
         QT_TR_NOOP("_Result Bias (added to each result pixel):"),
         -99999, +99999); // arbitrary!
    diaElemFloat result_multiplier
        (&(_param->result_multiplier),
         QT_TR_NOOP("Result _Multiplier (each result pixel\n"
           "multiplied by this):"),
         -99999, +99999); // arbitrary!
    diaElemUInteger histogram_frame_interval
        (&(_param->histogram_frame_interval),
         QT_TR_NOOP("_Histogram every N frames (0 to disable):"), 0, 0x7fffffff);
    diaElemUInteger debug
        (&(_param->debug), QT_TR_NOOP("_Debugging settings (bits):"), 0, 0x7fffffff);

    diaElem * elems[] = { &tool, &input_type, &input_file, &load_bias,
                          &load_multiplier, &input_constant,
                          &memory_constant_alpha, &lookahead_n_frames,
                          &init_start_frame, &init_end_frame,
                          &init_by_rolling, &bias, &result_bias,
                          &result_multiplier, &histogram_frame_interval,
                          &debug };

    ret = diaFactoryRun (QT_TR_NOOP("Swiss Army Knife Configuration"),
                         sizeof (elems) / sizeof (diaElem *), elems);
    if (ret) // 0 = cancel
    {
        myInfo->image_data_invalid = true;
        myInfo->histogram_data_invalid = true;
    }

    _param->input_file = file;
    delete[] file;

    return ret;
}

ADMVideoSwissArmyKnife::~ADMVideoSwissArmyKnife()
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
    printf ("ADMVideoSwissArmyKnife dtor (%p), conf = %p, pi = %p, rc now %d\n",
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
    delete _uncompressed;
    _uncompressed = NULL;
}

char * ADMVideoSwissArmyKnife::printConf ()
{
    return getConf (_param, false);
}

char * ADMVideoSwissArmyKnife::getConf (SWISSARMYKNIFE_PARAM * param,
                                        bool forDialog)
{
    const int CONF_LEN = 1024;
    static char conf[CONF_LEN];

    ToolMap * tm;
    for (tm = tool_map; tm->outputName; tm++)
        if (tm->toolid == param->tool)
            break;

    char inputstr [CONF_LEN];
    char where [256];
    where[0] = '\0';
    char moreinfo [256];
    moreinfo[0] = '\0';
    const char * input_file = param->input_file.c_str();
    if (!input_file || !*input_file)
        input_file = "**** no file selected ****";

    const char * space = forDialog ? " " : "";
    char * cptr;

    switch (param->input_type)
    {
    case INPUT_CUSTOM_CONVOLUTION:
        snprintf (inputstr, CONF_LEN, "convolve%s(P,%s%s)",
                  space, space, input_file);
        break;
    case INPUT_FILE_IMAGE_FLOAT:
        snprintf (inputstr, CONF_LEN,
                  (param->load_bias == 0.0 && param->load_multiplier == 1.0)
                  ? "pixel_from%s(%s)"
                  : (forDialog ? "((pixel_from%s(%s) + %.6f) * %.6f)"
                     : "((pixel_from%s(%s)%+.6f)*%.6f)"),
                  space, input_file,
                  param->load_bias, param->load_multiplier);
        break;
    case INPUT_FILE_IMAGE_INTEGER:
        snprintf (inputstr, CONF_LEN,
                  (param->load_bias == 0.0 && param->load_multiplier == 1.0)
                  ? "integer(pixel_from%s(%s))"
                  : (forDialog ? "integer((pixel_from%s(%s) + %.6f) * %.6f)"
                     : "integer((pixel_from%s(%s)%+.6f)*%.6f)"),
                  space, input_file,
                  param->load_bias, param->load_multiplier);
        break;
    case INPUT_CONSTANT_VALUE:
        if (int (param->input_constant) == param->input_constant)
            sprintf (inputstr, "%d", int (param->input_constant));
        else
            sprintf (inputstr, "%.6f", param->input_constant);
        break;
    case INPUT_ROLLING_AVERAGE:
        sprintf (inputstr, "A");
        cptr = where + sprintf (where, " (where each frame, "
                                "A=(A*(1-alpha))+(P*alpha), alpha%s=%s%.6f",
                                space, space, param->memory_constant_alpha);
        if (param->lookahead_n_frames)
            sprintf (cptr, ", P = pixel from %d frames ahead)",
                     param->lookahead_n_frames);
        else
            strcpy (cptr, ")");
        if (param->init_start_frame <= param->init_end_frame)
            sprintf (moreinfo, ", initial A = %s avg of frames %u - %u",
                     param->init_by_rolling ? "rolling" : "straight",
                     param->init_start_frame, param->init_end_frame);
        break;
    default:
        sprintf (inputstr, "OOOPS!! (unexpected type %d)",
                 param->input_type);
        break;
    }

    cptr = conf;

    bool result_is_scaled
        = (param->result_bias != 0.0 || param->result_multiplier != 1.0);

    if (!forDialog)
        cptr += snprintf (conf, CONF_LEN, "Swiss Army Knife: ");
    cptr += snprintf (cptr, CONF_LEN - (cptr - conf), "P' = %s",
                      result_is_scaled ? "((" : "");
    cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                      forDialog ? tm->spacy_format : tm->format, inputstr);
    if (param->bias)
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          forDialog ? " + %d" : "%+d", param->bias);
    cptr += snprintf (cptr, CONF_LEN - (cptr - conf), "%s", where);
    if (result_is_scaled)
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          forDialog ? ") + %.6f) * %.6f" : ")%+.6f)*%.6f",
                          param->result_bias, param->result_multiplier);
    cptr += snprintf (cptr, CONF_LEN - (cptr - conf), "%s", moreinfo);
    if (param->histogram_frame_interval)
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          ", histogram every %u frames",
                          param->histogram_frame_interval);
    if (param->debug)
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          ", debug%s=%s0x%x", space, space, param->debug);
    if (!forDialog)
        fprintf (stderr, "SAK conf is (%d) \"%s\"\n", cptr - conf, conf);
    return conf;
}

//============================================================================

// Note: The structure of the following code (with the template functions, and
// the functor objects, etc.) is all about minimizing the code executed
// per-pixel.  We need to select an input (convolution, etc.), and an
// operation to apply that input to each pixel (P' = P - A, etc.), and
// optionally scale the result, and optionally collect a histogram...but we
// don't want any more if's or pointer (or reference dereferences) in the
// per-pixel core of the loop than we absolutely must have, and we certainly
// don't want any switches or function calls.  At the same time, we don't want
// to duplicate the code that humans have to deal with - that's the whole
// point of templates.  So, buckle your seat belts, because this is where C++
// gets really fun!

//============================================================================

class HistogramNull // do-nothing version
{
public:
    static
    void reset()
    {
    }
    static
    void record_input (uint8_t P)
    {
    }
    static
    void record_output (int32_t P)
    {
    }
    static
    void dump (uint32_t frame_count, uint32_t pixels_per_frame)
    {
    }
    static
    bool frame_check ()
    {
        return false;
    }
};

//----------------------------------------------------------------------------

class Histogram
{
public:
    // Optionally, we could make this a template class with range_size a
    // template parameter.  However, it would have to be used in more than one
    // place, with different range sizes, for that to be worth doing.

    static const int32_t input_range_size = 256; // 8 bit pixels
    static const int32_t range_size = 1024; // This must be a power of 2!
    static const int32_t midpoint = 128;
    static const int32_t range_min = midpoint - (range_size / 2);
    static const int32_t range_max = midpoint + (range_size / 2) - 1;
    static const int32_t out_of_range_mask = ~(range_size - 1);

    uint32_t * input_data;
    uint32_t * output_data;
    uint32_t pixels_per_frame;
    uint32_t & frame_count;
    uint32_t frame_interval;

    // The default copy ctor and assignment operators are just fine in this
    // case!  Also, note that the ctor allocates the memory and sets the
    // pointers to which it is passed references, but does not keep the
    // references (since it won't need to change those pointers again) - it
    // keeps local copies instead, to avoid later indirections.  It does store
    // a reference to the frame_count for use in frame_check().

    Histogram (uint32_t * & input_data_ref, uint32_t * & output_data_ref,
               uint32_t frame_interval, uint32_t & frame_count,
               uint32_t pixels_per_frame)
        : input_data (input_data_ref),
          output_data (output_data_ref),
          pixels_per_frame (pixels_per_frame),
          frame_count (frame_count),
          frame_interval (frame_interval)
    {
        if (!input_data)
        {
            input_data = input_data_ref = new uint32_t [input_range_size];
            output_data = output_data_ref = new uint32_t [range_size];
            // printf ("histogram: allocated data at %p, %p\n",
            //         input_data, output_data);
            reset();
        }
        else
            ;// printf ("histogram: using data at %p, %p\n",
             //         input_data, output_data);
    }

    // no dtor needed - deallocation is handled elsewhere

    void reset () const
    {
        memset (input_data, 0, input_range_size * sizeof (input_data[0]));
        memset (output_data, 0, range_size * sizeof (output_data[0]));
        frame_count = 0;
    }

    void record_input (uint8_t P) const
    {
        input_data[P]++;
    }

    void record_output (int32_t P) const
    {
        P -= range_min; // scale into histogram data array index
        if ((P & out_of_range_mask) == 0)
        {
            output_data[P]++;
            return;
        }

        if (P < 0)
            P = 0;
        else if (P >= range_size)
            P = range_size - 1;

        output_data[P]++;
    }

    void dump () const
    {
        int32_t index_min = 0;
        int32_t index_max = range_size - 1;
        while (index_min < index_max)
        {
            if (output_data[index_min])
                break;
            ++index_min;
        }
        while (index_min < index_max)
        {
            if (output_data[index_max])
                break;
            --index_max;
        }
        
        printf ("Swiss Army Knife Histogram for past %u frames:\n"
                " =================== Input (0 - 255), "
                "avg/frame over %d frames: ====================\n",
                frame_count, frame_count);
        do_dump (input_data, 0, 255, 0);
        printf ("=========== Result before saturation (%d - %d), "
                "avg/frame over %d frames: ==========\n",
                index_min + range_min, index_max + range_min, frame_count);
        do_dump (output_data, index_min, index_max, range_min);
    }

    // returns true if it outputs data and resets the counter, else false.

    bool frame_check () const
    {
        if (++frame_count < frame_interval)
        {
            // printf ("histogram: frame %d of %d\n",
            //         frame_count, frame_interval);
            return false;
        }

        dump();
        reset();
        return true;
    }

private:
    void do_dump (uint32_t * data, int32_t index_min, int32_t index_max,
                  int32_t bias) const
    {
        // We scan the data to find the maximum value, so we can scale the
        // bars to provide useful visual data.

        uint32_t max_pixels = 0;
        for (int32_t index = index_min; index <= index_max; index++)
        {
            if (data[index] > max_pixels)
                max_pixels = data[index];
        }

        // Effectively, for each pixel-value count, we divide by the frame
        // count to get the average number of times that pixel value occurred
        // per frame, and then we scale the bar so that only the max number of
        // pixels gets 100% of the bar.

        const int32_t bar_max = 11; // hardcoded as %11s in printf below
#if 0
        max_pixels /= frame_count;
        int32_t divisor = frame_count * max_pixels / bar_max;
#else
        int32_t divisor = max_pixels / bar_max;
#endif

        const int32_t columns = 4;
        // The "+ columns" below is to round up to the total number of rows we
        // need including partial rows; it's really "+ 1 + (columns - 1)",
        // where the + 1 is to account for index_max being the top end of the
        // range, not the top end + 1.
        int32_t column_len = (index_max - index_min + columns) / columns;

        int32_t column_end = index_min + column_len;

        const char * bar = "*********************************************";
        const char * bar_end = bar + strlen (bar);

        for (int32_t index = index_min; index < column_end; index++)
        {
            int32_t index2 = index;
            int32_t val = index + bias;
            
            for (int32_t column = 0;
                 column < columns;
                 ++column, index2 += column_len, val += column_len)
            {
                if (index2 > index_max)
                    break;

                int32_t dat = data[index2];
                int32_t bar_size = dat / divisor;
                if (bar_size == 0 && dat != 0)
                    bar_size = 1;
#define HISTOGRAM_SHOW_VALUES 1
#ifdef HISTOGRAM_SHOW_VALUES
                printf (" %5d: %6d %-11s", val, dat / frame_count, bar_end - bar_size);
#else
                printf (" %5d: %-11s", val, bar_end - bar_size);
#endif
            }
            printf ("\n");
        }
    }
};

//============================================================================
/*
template <typename Oper, typename Histo>
void ImageTool::convolve (const std::vector <float> & kernel,
                          uint32_t kw, uint32_t kh, int32_t bias,
                          const Oper & op_in, const Histo & histogram_in)
{
    // We make local copies of the functors so that the calls below to
    // record_input() and record_output() (for the histogram) and operator()
    // (for the op) are accessing stack data rather than incurring yet another
    // indirection.  When it's per-pixel, every little bit helps!

    Histo histogram = histogram_in;
    Oper op = op_in;

    uint32_t sathigh = 0;
    uint32_t satlow = 0;

    // The following code is copied with little significant change (mostly
    // just porting & performance tweaks, and adding some debugging output)
    // from ImageJ's Convolver::convolveFloat() function.  Blame them for
    // variable names like "uc". ;-)

    // We have (for the moment) skipped the normalizing step - we assume that
    // the scale is 1.0.

    // We also assume that kw and kh are both odd.

    int32_t uc = kw / 2;
    int32_t vc = kh / 2;

    uint32_t xedge = my_w - uc;
    uint32_t yedge = my_h - vc;

    for (uint32_t y = 0; y < my_h; y++)
    {
        bool y_is_edgy = (y < vc || y >= yedge);

        for (uint32_t x = 0; x < my_w; x++)
        {
            float sum = 0;
            uint32_t i = 0;

            // If some of this pixel's neighbors are "off the edge" of the
            // input image, we'll use the "safe" getPixel().

            if (y_is_edgy || x < uc || x >= xedge)
            {
                for (int32_t v = -vc; v <= vc; v++)
                    for (int32_t u = -uc; u <= uc; u++)
                        sum += getPixelSafely (x + u, y + v) * kernel [i++];
            }
            else
            {
                // HERE: we could optimize this a good bit by putting
                // &(getPixel(x - uc, y - vc)) into a pointer, then just
                // incrementing the pointer (and adding a precomputed (my_w -
                // uc * 2) to shift lines).  It violates the nice
                // encapsulation provided by ImageTool, but if/when we add a
                // SIMD version of this loop, that sure isn't going to use
                // GetPixel()... ;-)

                for (int32_t v = -vc; v <= vc; v++)
                {
                    int32_t offset = x + ((y + v) * my_w);

                    for (int32_t u = -uc; u <= uc; u++)
                        sum += getPixel (offset + u) * kernel [i++];
                }
            }

            // as noted above, we're currently assuming the matrix was already
            // normalized...
            // sum *= scale;

            int32_t P = getPixel (x, y);

            histogram.record_input (P);

            float A = sum;

            int32_t result = op (P, A) + bias;

            histogram.record_output (result);

            if (result & 0xffffff00)
            {
                if (result < 0)
                {
                    result = 0;
                    satlow++;
                }
                else // if (result > 255)
                {
                    result = 255;
                    sathigh++;
                }
            }

            outPixel (x, y) = result;
        }
    }

    if (debug & 2)
    {
        if (satlow || sathigh)
            printf ("    Saturated %d low, %d high\n", satlow, sathigh);
    }
}
*/
//============================================================================

template <typename Oper, typename Histo>
void ADMVideoSwissArmyKnife::computeRollingAverage (ADMImage * image,
                                                    ADMImage * data,
                                                    uint32_t planesize,
                                                    SWISSARMYKNIFE_PARAM * param,
                                                    int32_t bias,
                                                    const Oper & op_in,
                                                    const Histo & histogram_in)
{
    // We make local copies of the functors so that the calls below to
    // record_input() and record_output() (for the histogram) and operator()
    // (for the op) are accessing stack data rather than incurring yet another
    // indirection.  When it's per-pixel, every little bit helps!

    Histo histogram = histogram_in;
    Oper op = op_in;

    float alpha = param->memory_constant_alpha;
    float oneminusalpha = 1 - alpha;

    // HERE: for speed, we do luma (Y plane) only.  However, some
    // users might want chroma, too... we should make that
    // an option or something.

    uint8_t * currp = YPLANE (image) + planesize;
    uint8_t * destp = YPLANE (data) + planesize;
    float * bgp = myInfo->bg + planesize;
    uint32_t pixremaining = planesize + 1;

    uint32_t sathigh = 0;
    uint32_t satlow = 0;

    while (--pixremaining)
    {
        int32_t P = *--currp;
        histogram.record_input (P);

        float A = *--bgp;
        *bgp = (A * oneminusalpha) + (P * alpha);

        int32_t result = op (P, A) + bias;

        histogram.record_output (result);

        if (result & 0xffffff00)
        {
            if (result < 0)
            {
                result = 0;
                satlow++;
            }
            else // if (result > 255)
            {
                result = 255;
                sathigh++;
            }
        }

        *--destp = result;
    }

    if (param->debug & 2)
    {
        if (satlow || sathigh)
            printf ("    Saturated %d low, %d high\n", satlow, sathigh);
    }
}

//============================================================================

// This one is used in the lookahead case; it takes the additional parameter
// supplying the lookahead frame.

template <typename Oper, typename Histo>
void ADMVideoSwissArmyKnife::computeRollingAverage (ADMImage * image,
                                                    ADMImage * lookaheadimage,
                                                    ADMImage * data,
                                                    uint32_t planesize,
                                                    SWISSARMYKNIFE_PARAM * param,
                                                    int32_t bias,
                                                    const Oper & op_in,
                                                    const Histo & histogram_in)
{
    // We make local copies of the functors so that the calls below to
    // record_input() and record_output() (for the histogram) and operator()
    // (for the op) are accessing stack data rather than incurring yet another
    // indirection.  When it's per-pixel, every little bit helps!

    Histo histogram = histogram_in;
    Oper op = op_in;

    float alpha = param->memory_constant_alpha;
    float oneminusalpha = 1 - alpha;

    // HERE: for speed, we do luma (Y plane) only.  However, some
    // users might want chroma, too... we should make that
    // an option or something.

    uint8_t * currp = YPLANE (image) + planesize;
    uint8_t * destp = YPLANE (data) + planesize;
    float * bgp = myInfo->bg + planesize;
    uint32_t pixremaining = planesize + 1;

    uint32_t sathigh = 0;
    uint32_t satlow = 0;

    if (lookaheadimage)
    {
        uint8_t * aheadp = YPLANE (lookaheadimage) + planesize;

        while (--pixremaining)
        {
            int32_t P = *--currp;
            histogram.record_input (P);

            float A = *--bgp;
            *bgp = (A * oneminusalpha) + (*--aheadp * alpha);

            int32_t result = op (P, A) + bias;

            histogram.record_output (result);

            if (result & 0xffffff00)
            {
                if (result < 0)
                {
                    result = 0;
                    satlow++;
                }
                else // if (result > 255)
                {
                    result = 255;
                    sathigh++;
                }
            }

            *--destp = result;
        }
    }
    else
    {
        while (--pixremaining)
        {
            int32_t P = *--currp;
            histogram.record_input (P);

            float A = *--bgp;
            // no update of background - the lookahead is looking past the
            // end of the video

            int32_t result = op (P, A) + bias;

            histogram.record_output (result);

            if (result & 0xffffff00)
            {
                if (result < 0)
                {
                    result = 0;
                    satlow++;
                }
                else // if (result > 255)
                {
                    result = 255;
                    sathigh++;
                }
            }

            *--destp = result;
        }
    }

    if (param->debug & 2)
    {
        if (satlow || sathigh)
            printf ("    Saturated %d low, %d high\n", satlow, sathigh);
    }
}

//============================================================================

template <typename InputImageType, typename Oper, typename Histo>
void ADMVideoSwissArmyKnife::applyImage (ADMImage * image, ADMImage * data,
                                         uint32_t planesize,
                                         SWISSARMYKNIFE_PARAM * param,
                                         int32_t bias,
                                         InputImageType * input_image,
                                         const Oper & op_in,
                                         const Histo & histogram_in)
{
    // We make local copies of the functors so that the calls below to
    // record_input() and record_output() (for the histogram) and operator()
    // (for the op) are accessing stack data rather than incurring yet another
    // indirection.  When it's per-pixel, every little bit helps!

    Histo histogram = histogram_in;
    Oper op = op_in;

    // HERE: for speed, we do luma (Y plane) only.  However, some
    // users might want chroma, too... we should make that
    // an option or something.

    uint8_t * currp = YPLANE (image) + planesize;
    uint8_t * destp = YPLANE (data) + planesize;
    InputImageType * bgp = input_image + planesize;
    uint32_t pixremaining = planesize + 1;

    uint32_t sathigh = 0;
    uint32_t satlow = 0;

    while (--pixremaining)
    {
        int32_t P = *--currp;
        histogram.record_input (P);

        InputImageType A = *--bgp;
        int32_t result = op (P, A) + bias;

        histogram.record_output (result);

        if (result & 0xffffff00)
        {
            if (result < 0)
            {
                result = 0;
                satlow++;
            }
            else // if (result > 255)
            {
                result = 255;
                sathigh++;
            }
        }

        *--destp = result;
    }

    if (param->debug & 2)
    {
        if (satlow || sathigh)
            printf ("    Saturated %d low, %d high\n", satlow, sathigh);
    }
}

//============================================================================

template <typename Oper, typename Histo>
void ADMVideoSwissArmyKnife::applyConstant (ADMImage * image, ADMImage * data,
                                            uint32_t planesize,
                                            SWISSARMYKNIFE_PARAM * param,
                                            int32_t bias,
                                            const Oper & op_in,
                                            const Histo & histogram_in)
{
    // We make local copies of the functors so that the calls below to
    // record_input() and record_output() (for the histogram) and operator()
    // (for the op) are accessing stack data rather than incurring yet another
    // indirection.  When it's per-pixel, every little bit helps!

    Histo histogram = histogram_in;
    Oper op = op_in;

    // HERE: for speed, we do luma (Y plane) only.  However, some
    // users might want chroma, too... we should make that
    // an option or something.

    uint8_t * currp = YPLANE (image) + planesize;
    uint8_t * destp = YPLANE (data) + planesize;
    float A = param->input_constant;
    uint32_t pixremaining = planesize + 1;

    uint32_t sathigh = 0;
    uint32_t satlow = 0;

    while (--pixremaining)
    {
        int32_t P = *--currp;
        histogram.record_input (P);

        int32_t result = op (P, A) + bias;

        histogram.record_output (result);

        if (result & 0xffffff00)
        {
            if (result < 0)
            {
                result = 0;
                satlow++;
            }
            else // if (result > 255)
            {
                result = 255;
                sathigh++;
            }
        }

        *--destp = result;
    }

    if (param->debug & 2)
    {
        if (satlow || sathigh)
            printf ("    Saturated %d low, %d high\n", satlow, sathigh);
    }
}

//============================================================================

class OpPequalsA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return static_cast <int32_t> (A + .5);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return A;
    }
};

class OpPequalsP
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return P;
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return P;
    }
};

class OpPequalsPminusA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return (P - static_cast <int32_t> (A + .5));
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return (P - A);
    }
};

class OpPequalsAminusP
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t> (A + .5) - P);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return (A - P);
    }
};

class OpPequalsPplusA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return (P + static_cast <int32_t> (A + .5));
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return (P + A);
    }
};

class OpPequalsPtimesA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return static_cast <int32_t> ((P * A) + .5);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return (P * A);
    }
};

class OpPequalsPdivByA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return static_cast <int32_t> ((P / A) + .5);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return (P / A);
    }
};

class OpPequalsAdivByP
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        return static_cast <int32_t> ((A / P) + .5);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return (A / P);
    }
};

class OpPequalsMinPA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        int32_t intA = static_cast <int32_t> (A + .5);
        return ((intA > P) ? P : intA);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return ((A > P) ? P : A);
    }
};

class OpPequalsMaxPA
{
public:
    int32_t operator () (int32_t P, float A) const
    {
        int32_t intA = static_cast <int32_t> (A + .5);
        return ((intA > P) ? intA : P);
    }

    int32_t operator () (int32_t P, uint8_t A) const
    {
        return ((A > P) ? A : P);
    }
};

//----------------------------------------------------------------------------

class OpPequalsA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return static_cast <int32_t> (((A + bias) * multiplier) + .5);
    }
};

class OpPequalsP_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsP_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return static_cast <int32_t> (((P + bias) * multiplier) + .5);
    }
};

class OpPequalsPminusA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsPminusA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((P - A) + bias) * multiplier) + .5));
    }
};

class OpPequalsAminusP_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsAminusP_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((A - P) + bias) * multiplier) + .5));
    }
};

class OpPequalsPplusA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsPplusA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((P + A) + bias) * multiplier) + .5));
    }
};

class OpPequalsPtimesA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsPtimesA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((P * A) + bias) * multiplier) + .5));
    }
};

class OpPequalsPdivByA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsPdivByA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((P / A) + bias) * multiplier) + .5));
    }
};

class OpPequalsAdivByP_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsAdivByP_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((A / P) + bias) * multiplier) + .5));
    }
};

class OpPequalsMinPA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsMinPA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((A > P ? P : A) + bias) * multiplier) + .5));
    }
};

class OpPequalsMaxPA_Scaled
{
    float bias;
    float multiplier;
public:
    OpPequalsMaxPA_Scaled (float bias, float multiplier)
        : bias (bias),
          multiplier (multiplier)  { }

    int32_t operator () (int32_t P, float A) const
    {
        return (static_cast <int32_t>
                ((((A > P ? A : P) + bias) * multiplier) + .5));
    }
};

//============================================================================

uint8_t ADMVideoSwissArmyKnife::getFrameNumberNoAlloc(uint32_t frame,
                                                      uint32_t *len,
                                                      ADMImage *data,
                                                      uint32_t *flags)
{
    if (frame >= _info.nb_frames)
        return 0;

    if (_param->debug & 1)
        printf ("in ADMVideoSwissArmyKnife::getFrameNumberNoAlloc(%d, ...)\n",
                frame);

    if (!_in->getFrameNumberNoAlloc (frame, len, _uncompressed, flags))
        return 0;

    uint32_t planesize = _info.width * _info.height;
    uint32_t size = (planesize * 3) >> 1;
    ADMImage * lookaheadimage;

    if (_param->lookahead_n_frames)
    {
        if (myInfo->bg_lab_size != planesize)
        {
            delete myInfo->bg_lab;
            myInfo->bg_lab_size = planesize;
            myInfo->bg_lab = new ADMImage (_info.width, _info.height);
        }

        if (frame + _param->lookahead_n_frames >= _info.nb_frames)
            lookaheadimage = 0;
        else if (!_in->getFrameNumberNoAlloc (frame
                                              + _param->lookahead_n_frames,
                                              len, myInfo->bg_lab, flags))
            return 0;
        else
            lookaheadimage = myInfo->bg_lab;
    }
    else
        lookaheadimage = 0;

    // printf ("%s\n", getConf (_param, true));

    *len = size;

    uint8_t ret = doSwissArmyKnife (_uncompressed, lookaheadimage,
                                    data, _in, this, _param,
                                    _info.width, _info.height);
    return ret;
}

uint8_t
ADMVideoSwissArmyKnife::doSwissArmyKnife (ADMImage * image,
                                          ADMImage * lookaheadimage,
                                          ADMImage * data,
                                          AVDMGenericVideoStream * in,
                                          ADMVideoSwissArmyKnife * sak,
                                          SWISSARMYKNIFE_PARAM * param,
                                          uint32_t width, uint32_t height)
{
    PersistentInfo * myInfo = sak->myInfo;
    uint32_t debug = param->debug;

    bool doingConvolution = (param->input_type == INPUT_CUSTOM_CONVOLUTION);
    bool doingRollingAvg = (param->input_type == INPUT_ROLLING_AVERAGE);
    bool doingFileImage = (param->input_type == INPUT_FILE_IMAGE_FLOAT
                           || param->input_type == INPUT_FILE_IMAGE_INTEGER);
    bool doingFileImageFloat = (param->input_type == INPUT_FILE_IMAGE_FLOAT);
    bool doingApplyConstant = (param->input_type == INPUT_CONSTANT_VALUE);

    bool needRead = false;
    bool needFile = doingConvolution || doingFileImage;

    if (needFile)
    {
        if (myInfo->input_file_name != param->input_file)
        {
            myInfo->input_file_name = param->input_file;
            myInfo->input_file_mtime = 0;
            printf ("SwissArmyKnife: new input file has been selected: %s\n",
                    myInfo->input_file_name.c_str());
            needRead = true;
        }

        // HERE: we should rearrange a bit to move the stat() call out to the
        // ctor - we don't need to check it every frame!

        struct stat st;
        if (stat (myInfo->input_file_name.c_str(), &st) == -1)
        {
            perror (myInfo->input_file_name.c_str());
            return 0;
        }

        if (st.st_mtime != myInfo->input_file_mtime)
        {
            if (!needRead && myInfo->input_file_mtime)
                printf ("SwissArmyKnife: input file %s has been changed - "
                        "re-reading it\n", myInfo->input_file_name.c_str());
            needRead = true;
            myInfo->input_file_mtime = st.st_mtime;
        }
        else if (!needRead && doingFileImage)
        {
            if (myInfo->image_data_invalid
                || myInfo->image_bias != param->load_bias
                || myInfo->image_multiplier != param->load_multiplier)
            {
                needRead = true;
            }
        }
    }

    //FloatVector & kernel = myInfo->kernel;
    uint32_t & kernel_w = myInfo->kernel_w;
    uint32_t & kernel_h = myInfo->kernel_h;

    uint32_t planesize = width * height;
			
    if (needRead)
    {
        myInfo->histogram_data_invalid = true;

        if (doingConvolution)
        {
            //kernel.clear();
            kernel_w = 0;
            kernel_h = 0;

            using namespace std;

            const char * filename = myInfo->input_file_name.c_str();
            ifstream inputStream (filename);
            if (!inputStream)
            {
                fprintf (stderr, "SwissArmyKnife: can't open input file %s, "
                         "but it apparently does exist...(%d)\n",
                         filename, errno);
                return 0;
            }

#ifndef ASSUME_SQUARE_MATRIX
            // We read the dimensions as floats just in case they happen to be
            // written that way (no reason to punt) - however, we do expect
            // that anything to the right of the decimal is 0!

            float dimtmp;
            inputStream >> dimtmp;
            kernel_w = uint32_t (dimtmp);
            if (float (kernel_w) != dimtmp)
                printf ("SwissArmyKnife: %s: What exactly do you expect a "
                        "width of %f to mean?  Truncating to %d...\n",
                        filename, dimtmp, uint32_t (dimtmp));
            inputStream >> dimtmp;
            kernel_h = uint32_t (dimtmp);
            if (float (kernel_h) != dimtmp)
                printf ("SwissArmyKnife: %s: What exactly do you expect a "
                        "height of %f to mean?  Truncating to %d...\n",
                        filename, dimtmp, uint32_t (dimtmp));
            if (kernel_w < 1 || (kernel_w & 1) == 0
                || kernel_h < 1 || (kernel_h & 1) == 0)
            {
                printf ("SwissArmyKnife: %s: Can't handle a convolution "
                        "kernel with dimensions %dx%d - both dimensions "
                        "must be odd (and positive)\n",
                        filename, int (kernel_w), int (kernel_h));
                myInfo->input_file_mtime = 0;  // force re-read, avoid crash
                return 0;
            }
#endif
/*
            copy (istream_iterator <float> (inputStream),
                  istream_iterator <float> (),
                  back_inserter (kernel));

#ifdef ASSUME_SQUARE_MATRIX
            kernel_dim = uint32_t (sqrtf (kernel.size()));
            if (kernel_dim * kernel_dim != kernel.size())
            {
                if ((kernel_dim + 1) * (kernel_dim + 1) == kernel.size())
                    ++kernel_dim;
                else
                {
                    printf ("SwissArmyKnife: Can't determine matrix "
                            "dimensions to explain %d input values! "
                            "(sqrt(%d) = %f)\n",
                            kernel.size(), kernel.size(),
                            sqrt (kernel.size()));
                    kernel_dim = 0;
                    return 0;
                }
            }
#endif

            printf ("SwissArmyKnife: read %d convolution kernel values "
                    "from %s (%dx%d):\n",
                    kernel.size(), filename, kernel_w, kernel_h);
            if (debug & 8)
            {
                FloatVector::const_iterator kit = kernel.begin();
                int count = 0;
                while (kit != kernel.end())
                {
                    printf ("%.6f%c", *kit++,
                            (++count % kernel_w) ? ' ' : '\n');
                }
                if (count % kernel_w)
                    printf ("[incomplete?!]\n");
            } */
        }
        else // file image (not convolution)
        {
            ADM_assert (param->input_type == INPUT_FILE_IMAGE_FLOAT
                        || param->input_type == INPUT_FILE_IMAGE_INTEGER);

            const char * filename = myInfo->input_file_name.c_str();
            FILE * fin = fopen (filename, "rb");
            if (!fin)
            {
                fprintf (stderr, "SwissArmyKnife: can't open input file %s, "
                         "but it apparently does exist...(%d)\n",
                         filename, errno);
                return 0;
            }

            ADMVideoComputeAverage::FileHeader header;
            int nread = fread (&header, sizeof (header), 1, fin);
            if (nread != 1 || strncmp (header.magic, "DGCMimgF", 8) != 0)
            {
                fprintf (stderr, "SwissArmyKnife: %s does not appear to be a "
                         "valid DG/CM floating-point raw image file (produced "
                         "by the ComputeAverage filter)\n", filename);
                fclose (fin);
                return 0;
            }

            uint32_t width = header.width;
            uint32_t height = header.height;
            uint32_t pixelcount = width * height;

            if (width > 2000 || height > 2000)
            {
                fprintf (stderr, "SwissArmyKnife: invalid image dimensions "
                         "(%dx%d) in %s\n",
                         int (width), int (height), filename);
                fclose (fin);
                return 0;
            }

            myInfo->image_data_invalid = true;
            delete [] myInfo->image_float;
            delete [] myInfo->image_int;

            myInfo->image_float = new float [pixelcount];

            nread = fread (myInfo->image_float, sizeof (float), pixelcount, fin);
            fclose (fin);
            if (nread != pixelcount)
            {
                fprintf (stderr, "SwissArmyKnife: failed to read image data "
                         "(%ux%u = %u) from %s (got %u)\n",
                         width, height, pixelcount, filename, nread);
                delete [] myInfo->image_float;
                myInfo->image_float = 0;
                return 0;
            }

            printf ("SwissArmyKnife: successfully loaded image data "
                    "(%ux%u = %u) from %s\n",
                    width, height, pixelcount, filename);

            myInfo->image_int = new uint8_t [pixelcount];
            float * floatpixp = myInfo->image_float + pixelcount;
            uint8_t * intpixp = myInfo->image_int + pixelcount;

            if (param->load_bias == 0.0 && param->load_multiplier == 1.0)
            {
                printf ("Converting %u pixels to integer values (and keeping "
                        "the floats, too)\n", pixelcount);

                ++pixelcount;
                while (--pixelcount)
                {
                    *--intpixp = static_cast <uint8_t> (*--floatpixp + 0.5);
                }
            }
            else
            {
                float load_bias = param->load_bias;
                float load_multiplier = param->load_multiplier;

                printf ("applying P = (P + %.6f) * %.6f to %u pixels\n",
                        load_bias, load_multiplier, pixelcount);

                ++pixelcount;
                while (--pixelcount)
                {
                    float floatpix
                        = (*--floatpixp + load_bias) * load_multiplier;
                    *floatpixp = floatpix;

                    uint8_t intpix;
                    if (floatpix < 0)
                        intpix = 0;
                    else if (floatpix > 255)
                        intpix = 255;
                    else
                        intpix = static_cast <uint8_t> (floatpix + 0.5);

                    *--intpixp = intpix;
                }
            }

            myInfo->image_w = width;
            myInfo->image_h = height;
            myInfo->image_bias = param->load_bias;
            myInfo->image_multiplier = param->load_multiplier;
            myInfo->image_data_invalid = false;
        }
    }
    else if (doingRollingAvg)
    {
        if (!myInfo->bg
            || myInfo->bg_x != width || myInfo->bg_y != height
            || myInfo->bg_mca != param->memory_constant_alpha
            || myInfo->bg_lanf != param->lookahead_n_frames
            || myInfo->bg_isf != param->init_start_frame
            || myInfo->bg_ief != param->init_end_frame
            || myInfo->bg_ibr != param->init_by_rolling)
        {
            if (!myInfo->bg || myInfo->bg_x != width || myInfo->bg_y != height)
            {
                myInfo->bg_x = width;
                myInfo->bg_y = height;
                delete [] myInfo->bg;
                myInfo->bg = new float [planesize];
            }

            myInfo->histogram_data_invalid = true;

            myInfo->bg_mca = param->memory_constant_alpha;
            myInfo->bg_lanf = param->lookahead_n_frames;
            myInfo->bg_isf = param->init_start_frame;
            myInfo->bg_ief = param->init_end_frame;
            myInfo->bg_ibr = param->init_by_rolling;

            if (myInfo->bg_isf <= myInfo->bg_ief)
            {
                uint32_t do_frames = myInfo->bg_ief - myInfo->bg_isf + 1;

                uint32_t firstframe = myInfo->bg_isf;
                uint32_t lastframe = myInfo->bg_ief;
                const ADV_Info & info = sak->getInfo();
                if (lastframe >= info.nb_frames)
                {
                    lastframe = info.nb_frames - 1;
                    if (firstframe >= info.nb_frames)
                    {
                        firstframe = lastframe - do_frames + 1;
                        if (firstframe >= info.nb_frames)
                            firstframe = 0;
                    }
                    do_frames = lastframe - firstframe + 1;
                }

                // if (debug & 2)
                printf ("Getting a \"head start\" on rolling average by "
                        "computing a %s %d frames of size %dx%d from %d to "
                        "%d with alpha = %.6f\n",
                        myInfo->bg_ibr ? "rolling average over"
                        : "straight average of",
                        do_frames, myInfo->bg_x, myInfo->bg_y,
                        firstframe, lastframe, myInfo->bg_mca);

                ADMImage aimage (myInfo->bg_x, myInfo->bg_y);

                // HERE: for speed, we do luma (Y plane) only.  However, some
                // users might want chroma, too... we should make that an option
                // or something.  (...in which case bg should be big enough for
                // all three planes.)


//***************************************************************************
//***************************************************************************
//***************************************************************************

                // HERE: issue: if we are not starting at the first frame,
                // then I think that "in->getFrameNumberNoAlloc(framenum)"
                // will give us framenum+frame_at_which_we_are_starting!!
                // not sure this matters a lot in practice right now, but it
                // might matter more if users start using a start frame > 1 to
                // get a good fore/aft ratio...

//***************************************************************************
//***************************************************************************
//***************************************************************************

                if (myInfo->bg_ibr)
                {
                    // We want to "prime" the rolling average with the values from
                    // the first frame (rather than zeroes, which doesn't seem to
                    // work out well).  So we do the following block just once,
                    // but in its own scope to not collide with the locals of the
                    // loop that follows:
                    {
                        uint32_t flen;
                        uint32_t fflags = 0;
                        if (!in->getFrameNumberNoAlloc (firstframe, &flen,
                                                        &aimage, &fflags))
                            return 0;

                        uint8_t * currp = YPLANE (&aimage) + planesize;
                        float * bgp = myInfo->bg + planesize;
                        uint32_t pixremaining = planesize + 1;
                        while (--pixremaining)
                        {
                            *--bgp = *--currp;
                        }

                        ++firstframe; // don't include this one again
                    }

                    float alpha = param->memory_constant_alpha;
                    float oneminusalpha = 1 - alpha;

                    for (int fnum = firstframe; fnum <= lastframe; fnum++)
                    {
                        uint32_t flen;
                        uint32_t fflags = 0;
                        if (!in->getFrameNumberNoAlloc (fnum, &flen,
                                                        &aimage, &fflags))
                            return 0;

                        uint8_t * currp = YPLANE (&aimage) + planesize;
                        float * bgp = myInfo->bg + planesize;
                        uint32_t pixremaining = planesize + 1;
                        while (--pixremaining)
                        {
                            --bgp;
                            *bgp = (*bgp * oneminusalpha) + (*--currp * alpha);
                        }
                    }
                }
                else // head start uses straight average (not rolling average)
                {
                    uint32_t sums [planesize];
                    memset (sums, 0, planesize * sizeof sums[0]);

                    for (int fnum = firstframe; fnum <= lastframe; fnum++)
                    {
                        uint32_t flen;
                        uint32_t fflags = 0;
                        if (!in->getFrameNumberNoAlloc (fnum, &flen,
                                                        &aimage, &fflags))
                            return 0;

                        uint8_t * currp = YPLANE (&aimage) + planesize;
                        uint32_t * sump = sums + planesize;
                        uint32_t pixremaining = planesize + 1;
                        while (--pixremaining)
                        {
                            *--sump += *--currp;
                        }
                    }

                    float * bgp = myInfo->bg + planesize;
                    uint32_t * sump = sums + planesize;
                    uint32_t pixremaining = planesize + 1;
                    // we use a floating point multiply of a reciprocal rather
                    // than a floating point divide, because floating point
                    // folklore says the multiply will be faster.
                    float one_over_framecount = 1.0 / do_frames;
                    while (--pixremaining)
                    {
                        *--bgp = *--sump * one_over_framecount;
                    }
                }

                // if (debug & 2)
                printf ("Done computing head start.\n");
            }
            else // if (debug & 2)
            {
                printf ("Starting with new 0 baseline background\n");
                memset (myInfo->bg, 0, planesize * sizeof (myInfo->bg[0]));
            }
        }
        else
        {
            if (debug & 4)
                printf ("Using existing baseline background of size %dx%d with "
                        "alpha = %.6f (1/%d)\n", myInfo->bg_x, myInfo->bg_y,
                        myInfo->bg_mca);
        }
    }			

    uint8_t * imagePixels = YPLANE (image);

    ImageTool imtool (imagePixels, width, height, data);
    imtool.setDebug (debug);

    Histogram * histogram = 0;
    int32_t bias = param->bias;
    uint32_t tool = param->tool;

    // HERE: give some thought to the general issue of keeping myInfo & param
    // in sync, and resetting the histogram (or whatever) when things change.

    if (param->histogram_frame_interval != 0)
    {
        histogram = new Histogram (myInfo->histogram_input_data,
                                   myInfo->histogram_output_data,
                                   param->histogram_frame_interval,
                                   myInfo->histogram_frame_count,
                                   width * height);
        tool += TOOL_ADD_HISTOGRAM;
        if (myInfo->histogram_frame_interval !=
            param->histogram_frame_interval)
        {
            if (myInfo->histogram_frame_interval)
                myInfo->histogram_data_invalid = true;
            myInfo->histogram_frame_interval
                = param->histogram_frame_interval;
        }

        if (myInfo->histogram_data_invalid)
        {
            myInfo->histogram_data_invalid = false;
            histogram->reset();
        }
    }

    float rbias = param->result_bias;
    float rmultiplier = param->result_multiplier;
    if (fabsf (rbias) >= 0.0001 || fabsf (rmultiplier - 1.0) >= 0.0001)
    {
        tool += TOOL_ADD_SCALING;
    }

    // It might look like the following switches could be collapsed
    // significantly if we just used a pointer to the functor objects,
    // assigning the appropriate operation, histogram functor, etc., and then
    // using fewer cases.  The problem is that to get the fastest possible
    // code (important since we're talking about per-pixel operations here),
    // the tool functor object (e.g., OpPequalsAminusP) needs to be
    // instantiated inline in the call to the operation template (e.g.,
    // convolve(), computeRollingAverage(), etc.) so that the functor code can
    // be inlined.  If we passed an object through a pointer or reference,
    // we'd have much smaller code (both source and binary), but it would also
    // be much slower because every pixel would have to traverse the pointer.
    // So in this case we are buying performance at the cost of a bunch of big
    // messy switches in the source plus massive template code expansion in
    // the output.  It's worth it.

    if (doingConvolution)
    {
        /*if (kernel.empty())
        {
            fprintf (stderr, "No convolution kernel loaded - can't do "
                     "convolution!\n");
            return 0;
        }*/

        switch (tool)
        {
        case TOOL_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsA(), HistogramNull());
            break;
        case TOOL_P: // HERE: we could optimize this if we wanted to
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsP(), HistogramNull());
            break;
        case TOOL_P_MINUS_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPminusA(), HistogramNull());
            break;
        case TOOL_A_MINUS_P:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsAminusP(), HistogramNull());
            break;
        case TOOL_P_PLUS_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPplusA(), HistogramNull());
            break;
        case TOOL_P_TIMES_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPtimesA(), HistogramNull());
            break;
        case TOOL_P_DIVBY_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPdivByA(), HistogramNull());
            break;
        case TOOL_A_DIVBY_P:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsAdivByP(), HistogramNull());
            break;
        case TOOL_MIN_P_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsMinPA(), HistogramNull());
            break;
        case TOOL_MAX_P_A:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsMaxPA(), HistogramNull());
            break;

        case TOOL_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsA(), *histogram);
            break;
        case TOOL_P_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsP(), *histogram);
            break;
        case TOOL_P_MINUS_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPminusA(), *histogram);
            break;
        case TOOL_A_MINUS_P_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsAminusP(), *histogram);
            break;
        case TOOL_P_PLUS_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPplusA(), *histogram);
            break;
        case TOOL_P_TIMES_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPtimesA(), *histogram);
            break;
        case TOOL_P_DIVBY_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsPdivByA(), *histogram);
            break;
        case TOOL_A_DIVBY_P_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsAdivByP(), *histogram);
            break;
        case TOOL_MIN_P_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsMinPA(), *histogram);
            break;
        case TOOL_MAX_P_A_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias, OpPequalsMaxPA(), *histogram);
            break;

        case TOOL_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsA_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_P_SCALED: // HERE_SCALED: we could optimize this if we wanted to
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsP_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_P_MINUS_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPminusA_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_A_MINUS_P_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsAminusP_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_P_PLUS_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPplusA_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_P_TIMES_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPtimesA_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_P_DIVBY_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPdivByA_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_A_DIVBY_P_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsAdivByP_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_MIN_P_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsMinPA_Scaled (rbias, rmultiplier), HistogramNull());
            break;
        case TOOL_MAX_P_A_SCALED:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsMaxPA_Scaled (rbias, rmultiplier), HistogramNull());
            break;

        case TOOL_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsA_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_P_SCALED_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsP_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_P_MINUS_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPminusA_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_A_MINUS_P_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsAminusP_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_P_PLUS_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPplusA_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_P_TIMES_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPtimesA_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_P_DIVBY_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsPdivByA_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_A_DIVBY_P_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsAdivByP_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_MIN_P_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsMinPA_Scaled (rbias, rmultiplier), *histogram);
            break;
        case TOOL_MAX_P_A_SCALED_WITH_HISTOGRAM:
            //imtool.convolve (kernel, kernel_w, kernel_h, bias,
                             //OpPequalsMaxPA_Scaled (rbias, rmultiplier), *histogram);
            break;

        default:
            fprintf (stderr, "SwissArmyKnife: unknown operation (tool) %d!\n",
                     param->tool);
            return 0;
        }
    }
    else if (doingRollingAvg && param->lookahead_n_frames == 0)
    {
        switch (tool)
        {
        case TOOL_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsA(), HistogramNull());
            break;
        case TOOL_P: // HERE: we could optimize this if we wanted to
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsP(), HistogramNull());
            break;
        case TOOL_P_MINUS_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPminusA(), HistogramNull());
            break;
        case TOOL_A_MINUS_P:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAminusP(), HistogramNull());
            break;
        case TOOL_P_PLUS_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPplusA(), HistogramNull());
            break;
        case TOOL_P_TIMES_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPtimesA(), HistogramNull());
            break;
        case TOOL_P_DIVBY_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPdivByA(), HistogramNull());
            break;
        case TOOL_A_DIVBY_P:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAdivByP(), HistogramNull());
            break;
        case TOOL_MIN_P_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMinPA(), HistogramNull());
            break;
        case TOOL_MAX_P_A:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMaxPA(), HistogramNull());
            break;

        case TOOL_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsA(), *histogram);
            break;
        case TOOL_P_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsP(), *histogram);
            break;
        case TOOL_P_MINUS_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPminusA(), *histogram);
            break;
        case TOOL_A_MINUS_P_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAminusP(), *histogram);
            break;
        case TOOL_P_PLUS_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPplusA(), *histogram);
            break;
        case TOOL_P_TIMES_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPtimesA(), *histogram);
            break;
        case TOOL_P_DIVBY_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPdivByA(), *histogram);
            break;
        case TOOL_A_DIVBY_P_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAdivByP(), *histogram);
            break;
        case TOOL_MIN_P_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMinPA(), *histogram);
            break;
        case TOOL_MAX_P_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMaxPA(), *histogram);
            break;

        case TOOL_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_SCALED: // HERE_SCALED: we could optimize this if we wanted to
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsP_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_MINUS_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPminusA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_A_MINUS_P_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAminusP_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_PLUS_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPplusA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_TIMES_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_DIVBY_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_A_DIVBY_P_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_MIN_P_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMinPA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_MAX_P_A_SCALED:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;

        case TOOL_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_SCALED_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsP_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_MINUS_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPminusA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_A_MINUS_P_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAminusP_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_PLUS_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPplusA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_TIMES_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_DIVBY_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_A_DIVBY_P_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_MIN_P_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMinPA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_MAX_P_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, data, planesize, param, bias,
                                        OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;

        default:
            fprintf (stderr, "SwissArmyKnife: unknown operation (tool) %d!\n",
                     param->tool);
            return 0;
        }
    }
    else if (doingRollingAvg)
    {
        switch (tool)
        {
        case TOOL_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsA(), HistogramNull());
            break;
        case TOOL_P: // HERE: we could optimize this if we wanted to
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsP(), HistogramNull());
            break;
        case TOOL_P_MINUS_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPminusA(), HistogramNull());
            break;
        case TOOL_A_MINUS_P:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsAminusP(), HistogramNull());
            break;
        case TOOL_P_PLUS_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPplusA(), HistogramNull());
            break;
        case TOOL_P_TIMES_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPtimesA(), HistogramNull());
            break;
        case TOOL_P_DIVBY_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPdivByA(), HistogramNull());
            break;
        case TOOL_A_DIVBY_P:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsAdivByP(), HistogramNull());
            break;
        case TOOL_MIN_P_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsMinPA(), HistogramNull());
            break;
        case TOOL_MAX_P_A:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsMaxPA(), HistogramNull());
            break;

        case TOOL_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsA(), *histogram);
            break;
        case TOOL_P_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsP(), *histogram);
            break;
        case TOOL_P_MINUS_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPminusA(), *histogram);
            break;
        case TOOL_A_MINUS_P_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsAminusP(), *histogram);
            break;
        case TOOL_P_PLUS_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPplusA(), *histogram);
            break;
        case TOOL_P_TIMES_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPtimesA(), *histogram);
            break;
        case TOOL_P_DIVBY_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsPdivByA(), *histogram);
            break;
        case TOOL_A_DIVBY_P_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsAdivByP(), *histogram);
            break;
        case TOOL_MIN_P_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsMinPA(), *histogram);
            break;
        case TOOL_MAX_P_A_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias, OpPequalsMaxPA(), *histogram);
            break;

        case TOOL_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_SCALED: // HERE_SCALED: we could optimize this if we wanted to
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsP_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_MINUS_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPminusA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_A_MINUS_P_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsAminusP_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_PLUS_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPplusA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_TIMES_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_P_DIVBY_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_A_DIVBY_P_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_MIN_P_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsMinPA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;
        case TOOL_MAX_P_A_SCALED:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                                        HistogramNull());
            break;

        case TOOL_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_SCALED_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsP_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_MINUS_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPminusA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_A_MINUS_P_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsAminusP_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_PLUS_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPplusA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_TIMES_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_P_DIVBY_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_A_DIVBY_P_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_MIN_P_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsMinPA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;
        case TOOL_MAX_P_A_SCALED_WITH_HISTOGRAM:
            sak->computeRollingAverage (image, lookaheadimage, data, planesize,
                                        param, bias,
                                        OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                                        *histogram);
            break;

        default:
            fprintf (stderr, "SwissArmyKnife: unknown operation (tool) %d!\n",
                     param->tool);
            return 0;
        }
    }
    else if (doingFileImageFloat)
    {
        if (width != myInfo->image_w || height != myInfo->image_h)
        {
            const char * bar = "*************************";
            fprintf (stderr, "\n%s%s%s\nAttempting to apply a %ux%u input "
                     "image to %ux%u video - even if I could do that, it "
                     "probably wouldn't be what you wanted...\n%s%s%s\n",
                     bar, bar, bar, myInfo->image_w, myInfo->image_h,
                     width, height, bar, bar, bar);
            return 0;
        }

        float * flt_img = myInfo->image_float;

        switch (tool)
        {
        case TOOL_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsA(), HistogramNull());
            break;
        case TOOL_P: // HERE: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsP(), HistogramNull());
            break;
        case TOOL_P_MINUS_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPminusA(), HistogramNull());
            break;
        case TOOL_A_MINUS_P:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAminusP(), HistogramNull());
            break;
        case TOOL_P_PLUS_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPplusA(), HistogramNull());
            break;
        case TOOL_P_TIMES_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPtimesA(), HistogramNull());
            break;
        case TOOL_P_DIVBY_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPdivByA(), HistogramNull());
            break;
        case TOOL_A_DIVBY_P:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAdivByP(), HistogramNull());
            break;
        case TOOL_MIN_P_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMinPA(), HistogramNull());
            break;
        case TOOL_MAX_P_A:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMaxPA(), HistogramNull());
            break;

        case TOOL_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsA(), *histogram);
            break;
        case TOOL_P_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsP(), *histogram);
            break;
        case TOOL_P_MINUS_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPminusA(), *histogram);
            break;
        case TOOL_A_MINUS_P_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAminusP(), *histogram);
            break;
        case TOOL_P_PLUS_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPplusA(), *histogram);
            break;
        case TOOL_P_TIMES_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPtimesA(), *histogram);
            break;
        case TOOL_P_DIVBY_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPdivByA(), *histogram);
            break;
        case TOOL_A_DIVBY_P_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAdivByP(), *histogram);
            break;
        case TOOL_MIN_P_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMinPA(), *histogram);
            break;
        case TOOL_MAX_P_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMaxPA(), *histogram);
            break;

        case TOOL_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_SCALED: // HERE_SCALED: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsP_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_MINUS_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPminusA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_A_MINUS_P_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAminusP_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_PLUS_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPplusA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_TIMES_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_DIVBY_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_A_DIVBY_P_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_MIN_P_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMinPA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_MAX_P_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;

        case TOOL_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_SCALED_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsP_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_MINUS_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPminusA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_A_MINUS_P_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAminusP_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_PLUS_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPplusA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_TIMES_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_DIVBY_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_A_DIVBY_P_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_MIN_P_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMinPA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_MAX_P_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, flt_img,
                             OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;

        default:
            fprintf (stderr, "SwissArmyKnife: unknown operation (tool) %d!\n",
                     param->tool);
            return 0;
        }
    }
    else if (doingFileImage)
    {
        if (width != myInfo->image_w || height != myInfo->image_h)
        {
            const char * bar = "*************************";
            fprintf (stderr, "\n%s%s%s\nAttempting to apply a %ux%u input "
                     "image to %ux%u video - even if I could do that, it "
                     "probably wouldn't be what you wanted...\n%s%s%s\n",
                     bar, bar, bar, myInfo->image_w, myInfo->image_h,
                     width, height, bar, bar, bar);
            return 0;
        }

        uint8_t * int_img = myInfo->image_int;

        switch (tool)
        {
        case TOOL_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsA(), HistogramNull());
            break;
        case TOOL_P: // HERE: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsP(), HistogramNull());
            break;
        case TOOL_P_MINUS_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPminusA(), HistogramNull());
            break;
        case TOOL_A_MINUS_P:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAminusP(), HistogramNull());
            break;
        case TOOL_P_PLUS_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPplusA(), HistogramNull());
            break;
        case TOOL_P_TIMES_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPtimesA(), HistogramNull());
            break;
        case TOOL_P_DIVBY_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPdivByA(), HistogramNull());
            break;
        case TOOL_A_DIVBY_P:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAdivByP(), HistogramNull());
            break;
        case TOOL_MIN_P_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMinPA(), HistogramNull());
            break;
        case TOOL_MAX_P_A:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMaxPA(), HistogramNull());
            break;

        case TOOL_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsA(), *histogram);
            break;
        case TOOL_P_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsP(), *histogram);
            break;
        case TOOL_P_MINUS_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPminusA(), *histogram);
            break;
        case TOOL_A_MINUS_P_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAminusP(), *histogram);
            break;
        case TOOL_P_PLUS_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPplusA(), *histogram);
            break;
        case TOOL_P_TIMES_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPtimesA(), *histogram);
            break;
        case TOOL_P_DIVBY_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPdivByA(), *histogram);
            break;
        case TOOL_A_DIVBY_P_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAdivByP(), *histogram);
            break;
        case TOOL_MIN_P_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMinPA(), *histogram);
            break;
        case TOOL_MAX_P_A_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMaxPA(), *histogram);
            break;

        case TOOL_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_SCALED: // HERE_SCALED: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsP_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_MINUS_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPminusA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_A_MINUS_P_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAminusP_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_PLUS_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPplusA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_TIMES_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_P_DIVBY_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_A_DIVBY_P_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_MIN_P_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMinPA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;
        case TOOL_MAX_P_A_SCALED:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                             HistogramNull());
            break;

        case TOOL_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_SCALED_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsP_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_MINUS_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPminusA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_A_MINUS_P_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAminusP_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_PLUS_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPplusA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_TIMES_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_P_DIVBY_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_A_DIVBY_P_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_MIN_P_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMinPA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;
        case TOOL_MAX_P_A_SCALED_WITH_HISTOGRAM:
            sak->applyImage (image, data, planesize, param, bias, int_img,
                             OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                             *histogram);
            break;

        default:
            fprintf (stderr, "SwissArmyKnife: unknown operation (tool) %d!\n",
                     param->tool);
            return 0;
        }
    }
    else if (doingApplyConstant)
    {
        switch (tool)
        {
        case TOOL_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsA(), HistogramNull());
            break;
        case TOOL_P: // HERE: we could optimize this if we wanted to
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsP(), HistogramNull());
            break;
        case TOOL_P_MINUS_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPminusA(), HistogramNull());
            break;
        case TOOL_A_MINUS_P:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAminusP(), HistogramNull());
            break;
        case TOOL_P_PLUS_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPplusA(), HistogramNull());
            break;
        case TOOL_P_TIMES_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPtimesA(), HistogramNull());
            break;
        case TOOL_P_DIVBY_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPdivByA(), HistogramNull());
            break;
        case TOOL_A_DIVBY_P:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAdivByP(), HistogramNull());
            break;
        case TOOL_MIN_P_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMinPA(), HistogramNull());
            break;
        case TOOL_MAX_P_A:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMaxPA(), HistogramNull());
            break;

        case TOOL_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsA(), *histogram);
            break;
        case TOOL_P_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsP(), *histogram);
            break;
        case TOOL_P_MINUS_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPminusA(), *histogram);
            break;
        case TOOL_A_MINUS_P_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAminusP(), *histogram);
            break;
        case TOOL_P_PLUS_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPplusA(), *histogram);
            break;
        case TOOL_P_TIMES_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPtimesA(), *histogram);
            break;
        case TOOL_P_DIVBY_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPdivByA(), *histogram);
            break;
        case TOOL_A_DIVBY_P_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAdivByP(), *histogram);
            break;
        case TOOL_MIN_P_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMinPA(), *histogram);
            break;
        case TOOL_MAX_P_A_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMaxPA(), *histogram);
            break;

        case TOOL_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_P_SCALED: // HERE_SCALED: we could optimize this if we wanted to
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsP_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_P_MINUS_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPminusA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_A_MINUS_P_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAminusP_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_P_PLUS_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPplusA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_P_TIMES_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_P_DIVBY_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_A_DIVBY_P_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_MIN_P_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMinPA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;
        case TOOL_MAX_P_A_SCALED:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                                HistogramNull());
            break;

        case TOOL_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_P_SCALED_WITH_HISTOGRAM: // HERE: we could optimize this if we wanted to
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsP_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_P_MINUS_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPminusA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_A_MINUS_P_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAminusP_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_P_PLUS_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPplusA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_P_TIMES_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPtimesA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_P_DIVBY_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsPdivByA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_A_DIVBY_P_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsAdivByP_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_MIN_P_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMinPA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;
        case TOOL_MAX_P_A_SCALED_WITH_HISTOGRAM:
            sak->applyConstant (image, data, planesize, param, bias,
                                OpPequalsMaxPA_Scaled (rbias, rmultiplier),
                                *histogram);
            break;

        default:
            fprintf (stderr, "SwissArmyKnife: unknown operation (tool) %d!\n",
                     param->tool);
            return 0;
        }
    }
    else
    {
        fprintf (stderr, "ooops!  input selection botch in SwissArmyKnife!\n");
        return 0;
    }

    if (histogram)
    {
        histogram->frame_check();
        delete histogram;
    }

    // HERE: the following two lines do a luma-only-ize

    memset (UPLANE (data), 128, planesize >> 2);
    memset (VPLANE (data), 128, planesize >> 2);

    data->copyInfo(image);

    return 1;
}	                           
