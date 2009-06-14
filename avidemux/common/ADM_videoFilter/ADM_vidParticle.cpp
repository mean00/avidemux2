/***************************************************************************
                          ADM_vidParticle.cpp  -  detect particles (groups
                                                     of pixels)
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
 
#include <math.h>
#include <algorithm>
#include <string>
#include <ctype.h>

using namespace std;

#include "ADM_default.h"

#undef memcpy   // avoid compile errors due to macro
#include "fourcc.h"

#include "ADM_toolkit/toolkit.hxx"
#include "ADM_videoFilter.h"

#include "ADM_encoder/ADM_vidEncode.hxx"
#include "ADM_encoder/adm_encoder.h"

#include "ADM_filter/video_filters.h"

#include "DIA_factory.h"

#include "ADM_vidParticle.h"
#include "ADM_userInterfaces/ADM_commonUI/DIA_flyDialog.h" // for MenuMapping

FILE * ADMVideoParticle::outfp = 0;
uint32_t ADMVideoParticle::last_frame_written = 0xffffffff;

static FILTER_PARAM particleParam =
{
    10,
    { "min_area", "max_area", "left_crop", "right_crop",          // 4
      "top_crop", "bottom_crop", "output_format", "output_file",  // + 4 = 8
//      "camera_number", "min_dim_pctile", "max_dim_pctile", "debug" // + 4 = 12
      "camera_number", "debug" // + 2 = 10
    }
};

SCRIPT_CREATE(particle_script,ADMVideoParticle,particleParam);

BUILD_CREATE(particle_create,ADMVideoParticle);

ADMVideoParticle::ADMVideoParticle (AVDMGenericVideoStream *in, CONFcouple *couples)
{
    printf ("ADMVideoParticle ctor (%p)\n", this);
    _in = in;
    memcpy(&_info, in->getInfo(), sizeof(_info));
    _info.encoding = 1;
    _uncompressed = new ADMImage(_in->getInfo()->width, _in->getInfo()->height);
    ADM_assert(_uncompressed);
    _param = new PARTICLE_PARAM;

    if (couples)
    {
        GET(min_area);
        GET(max_area);
        GET(left_crop);
        GET(right_crop);
        GET(top_crop);
        GET(bottom_crop);
        GET(output_format);

	char* tmp;
        GET2(output_file, tmp);
        GET(camera_number);
//        GET(min_dim_pctile);
//        GET(max_dim_pctile);
        GET(debug);
    }
    else
    {
        _param->min_area = 5;
        _param->max_area = 50000;
        _param->left_crop = 0;
        _param->right_crop = 0;
        _param->top_crop = 0;
        _param->bottom_crop = 0;
        _param->output_format = OUTPUTFMT_FORMAT_NEW;
        // _param->output_file = ""; // implicit
        _param->camera_number = 1;
//        _param->min_dim_pctile = 0;
//        _param->max_dim_pctile = 100;
        _param->debug = 0;
    }
}

uint8_t	ADMVideoParticle::getCoupledConf (CONFcouple **couples)
{

    ADM_assert(_param);
    *couples = new CONFcouple(particleParam.nb);

    CSET(min_area);
    CSET(max_area);
    CSET(left_crop);
    CSET(right_crop);
    CSET(top_crop);
    CSET(bottom_crop);
    CSET(output_format);
	(*couples)->setCouple("output_file", _param->output_file.c_str());
    CSET(camera_number);
//    CSET(min_dim_pctile);
//    CSET(max_dim_pctile);
    CSET(debug);

    return 1;

}

uint8_t ADMVideoParticle::configure (AVDMGenericVideoStream *in)
{
    diaMenuEntry tOutputFmt [] = {
        { OUTPUTFMT_FORMAT_NEW,
          QT_TR_NOOP("New format, preferred, good for Tracker3D"), NULL },
        { OUTPUTFMT_FORMAT_DG_UWA,
          QT_TR_NOOP("DG@UWA format (includes shape info & bounding box)"), NULL },
        { OUTPUTFMT_FORMAT_AB_ODU,
          QT_TR_NOOP("AB@ODU format (includes shape info)"), NULL },
        { OUTPUTFMT_FORMAT_OLD,
          QT_TR_NOOP("Old format that Tracker3D can't read directly"), NULL },
    };

    diaElemMenu output_format
        (&(_param->output_format), QT_TR_NOOP("Output _Format:"),
         sizeof (tOutputFmt) / sizeof (diaMenuEntry), tOutputFmt);

    MenuMapping menu_mapping [] = {
        { "outputFormatMenu", my_offsetof (PARTICLE_PARAM, output_format),
          sizeof (tOutputFmt) / sizeof (diaMenuEntry), tOutputFmt },
    };

    uint8_t ret = DIA_particle (_in, this, _param, menu_mapping,
                                sizeof (menu_mapping) / sizeof (MenuMapping));
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

    diaElemUInteger min_area
        (&(_param->min_area),
         QT_TR_NOOP("Mi_nimum area for a particle to be detected:"), 1, 0x7fffffff);
    diaElemUInteger max_area
        (&(_param->max_area),
         QT_TR_NOOP("Ma_ximum area for a particle to be detected:"), 1, 0x7fffffff);

    diaElemUInteger left_crop
        (&(_param->left_crop),
         QT_TR_NOOP("_Left side crop (ignore particles in):"), 0, 0x7fffffff);
    diaElemUInteger right_crop
        (&(_param->right_crop),
         QT_TR_NOOP("_Right side crop (ignore particles in):"), 0, 0x7fffffff);
    diaElemUInteger top_crop
        (&(_param->top_crop),
         QT_TR_NOOP("_Top crop (ignore particles in):"), 0, 0x7fffffff);
    diaElemUInteger bottom_crop
        (&(_param->bottom_crop),
         QT_TR_NOOP("_Bottom crop (ignore particles in):"), 0, 0x7fffffff);

	char* file = ADM_strdup(_param->output_file.c_str());

    diaElemFile output_file
        (1, &file, QT_TR_NOOP("_Output File:"), NULL, QT_TR_NOOP("Select file"));

    diaElemUInteger camera_number
        (&(_param->camera_number), QT_TR_NOOP("_Camera number:"), 1, 0x7fffffff);

    diaElemUInteger debug(&(_param->debug), QT_TR_NOOP("_Debugging settings (bits):"),
                          0, 0x7fffffff);

    diaElem * elems[] = { &min_area, &max_area, &left_crop, &right_crop,
                          &top_crop, &bottom_crop, &output_format,
                          &output_file, &camera_number, &debug };

    ret = diaFactoryRun ("Particle Detection Configuration",
                         sizeof (elems) / sizeof (diaElem *), elems);

	_param->output_file = file;
	delete[] file;

    return ret;
}

ADMVideoParticle::~ADMVideoParticle()
{
    printf ("ADMVideoParticle dtor (%p)\n", this);
    DELETE(_param);
    delete _uncompressed;
    _uncompressed = NULL;
}

char *ADMVideoParticle::printConf (void)
{
    const int CONF_LEN = 1024;
    static char conf[CONF_LEN];

    char * cptr = conf;

    cptr += snprintf (conf, CONF_LEN, "ParticleList: Area=%u..%u",
                      _param->min_area, _param->max_area);

    if (_param->left_crop || _param->right_crop || _param->top_crop
        || _param->bottom_crop)
    {
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          ", Crop (L/R/T/B): %u/%u/%u/%u",
                          _param->left_crop, _param->right_crop,
                          _param->top_crop, _param->bottom_crop);
    }

    if (!_param->output_file.empty())
    {
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf), ", %s Output to %s",
                          _param->output_format == OUTPUTFMT_FORMAT_OLD
                          ? "Old" : "New", _param->output_file.c_str());
    }

    cptr += snprintf (cptr, CONF_LEN - (cptr - conf), ", Camera # %u",
                      _param->camera_number);

    if (_param->debug)
        cptr += snprintf (cptr, CONF_LEN - (cptr - conf),
                          ", debug=0x%x", _param->debug);

    return conf;
}

uint8_t ADMVideoParticle::getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
                                                ADMImage *data, uint32_t *flags)
{
    if (frame >= _info.nb_frames)
        return 0;

    if (_param->debug & (0x01 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
        printf ("in ADMVideoParticle::getFrameNumberNoAlloc(%d, ...)\n", frame);

    if (!_in->getFrameNumberNoAlloc (frame, len, _uncompressed, flags))
        return 0;

    uint32_t planesize = _info.width * _info.height;
    uint32_t size = (planesize * 3) >> 1;
    *len = size;

    uint32_t real_frame = frame + _info.orgFrame;
    OutputFmt outfmt = static_cast <OutputFmt> (_param->output_format);

    if (!outfp && real_frame == 0 && !_param->output_file.empty())
    {
        printf ("Starting to write particle list to %s\n",
                _param->output_file.c_str());

        outfp = fopen (_param->output_file.c_str(), "w");
        if (!outfp)
        {
            perror (_param->output_file.c_str());
        }
        else if (real_frame == 0) // ick
        {
            switch (outfmt)
            {
            case OUTPUTFMT_FORMAT_OLD:
                fprintf (outfp, "%% Particle  Area  X  Y  Frame\n");
                break;

            case OUTPUTFMT_FORMAT_NEW:
                fprintf (outfp, "%% Frame  Camera  X  Y  Area  Particle\n");
                break;

            case OUTPUTFMT_FORMAT_DG_UWA:
                fprintf (outfp,
                         "%% Frame #, Camera #, X, Y, Area, Particle #, "
                         "Bounding Box Width, Bounding Box Height, "
                         "Min Dim, Max Dim, Min Dim Angle, Max Dim Angle, "
                         "Max / Min, Length, Length Angle, "
                         "Width, Width Angle, Length / Width\n");
                break;

            case OUTPUTFMT_FORMAT_AB_ODU:
#ifdef OLD_PARTICLE_WEDGE_SCHEME
                fprintf (outfp,
                         "Frame #,Particle #,Area,X,Y,Min Dim,Max Dim,"
                         "Min Dim Angle,Max Dim Angle,Min / Max\n");
#else // ! OLD_PARTICLE_WEDGE_SCHEME
                fprintf (outfp,
                         "Frame #,Particle #,Area,X,Y, Min Dim,Max Dim, "
                         "Min Dim Angle,Max Dim Angle, Max / Min,  "
                         "Length,Length Angle, Width,Width Angle, "
                         "Length / Width\n");
#endif // ? OLD_PARTICLE_WEDGE_SCHEME
                break;

            default:
                fprintf (outfp, "%% Uh oh, unknown output format %d!\n",
                         outfmt);
                break;
            }
        }
    }

    uint8_t ret = doParticle (_uncompressed, data, _in, real_frame,
                              (last_frame_written == real_frame) ? 0 : outfp,
                              _param, _info.width, _info.height);

    last_frame_written = real_frame;

    if (outfp && real_frame >= _info.nb_frames + _info.orgFrame - 1)
    {
        fclose (outfp);
        outfp = 0;
        fprintf (stderr, "Finished writing particle list to %s\n",
                 _param->output_file.c_str());
    }

    return ret;
}

uint8_t
ADMVideoParticle::doParticle (ADMImage * image, ADMImage * data,
                              AVDMGenericVideoStream * in,
                              uint32_t real_frame,
                              FILE * do_outfp, PARTICLE_PARAM * param,
                              uint32_t width, uint32_t height)
{
    uint32_t debug = param->debug;

    uint32_t planesize = width * height;
    uint32_t uvplanesize = planesize >> 2;
    uint32_t size = planesize + (uvplanesize * 2);
			
    memset (UPLANE (data), 128, uvplanesize);
    memset (VPLANE (data), 128, uvplanesize);
    memset (YPLANE (data), 0, planesize);

    // HERE: we should erase anything outside the ROI (region of interest),
    // unless we decide to handle that some other way and/or place...

    OutputFmt outfmt = static_cast <OutputFmt> (param->output_format);
    bool fancyOutput = (outfmt == OUTPUTFMT_FORMAT_AB_ODU ||
                        outfmt == OUTPUTFMT_FORMAT_DG_UWA);

    uint8_t * imagePixels = YPLANE (image);
    uint32_t imagetool_flags
        = fancyOutput ? ImageTool::FLAG_DO_SHAPE : ImageTool::FLAG_NONE;

    ImageTool imtool (imagePixels, width, height, data, imagetool_flags);
    imtool.setDebug (debug);
    imtool.setMinArea (param->min_area);
    imtool.setMaxArea (param->max_area);
    imtool.setCropping (param->left_crop, param->right_crop,
                        param->top_crop, param->bottom_crop);
    static uint32_t totalParticleNum = 0;
    uint32_t frameParticleNum = 0;

    uint32_t camera_number = param->camera_number;

    for (uint32_t y = param->top_crop; y < height - param->bottom_crop; y++)
    {
        uint8_t * pixelrow = imagePixels + (y * width);
        for (uint32_t x = param->left_crop; x < width - param->right_crop; x++)
        {
            if (imtool.goodPixel (pixelrow [x]))
            {
                if (imtool.autoOutline (x, y) == 0)
                    continue;

                const Particle & particle = imtool.getParticle();
                uint32_t area = particle.area;
                float centroid_x = particle.centroidX;
                float centroid_y = particle.centroidY;

                if (debug & (0x02 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
                    printf ("frame %d, particle %d (%d total), "
                            "%d pixels centered at (%.6f,%.6f)\n",
                            real_frame, frameParticleNum,
                            totalParticleNum, area, centroid_x, centroid_y);

                if (!do_outfp)
                    continue;

                ++totalParticleNum;
                ++frameParticleNum;

                if (fancyOutput)
                {
#ifdef OLD_PARTICLE_WEDGE_SCHEME
                    uint32_t mindim = particle.getMinDimCount();
                    uint32_t maxdim = particle.getMaxDimCount();
                    float mindimangle = particle.getMinDimAngle();
                    float maxdimangle = particle.getMaxDimAngle();
                    float minmaxratio = float (mindim) / float (maxdim);
#else // ! OLD_PARTICLE_WEDGE_SCHEME
                    float mindist = particle.getMinDistance();
                    float maxdist = particle.getMaxDistance();
                    float mindistangle = particle.getMinDistAngle();
                    float maxdistangle = particle.getMaxDistAngle();
                    float maxminratio = float (maxdist) / float (mindist);
                    const Particle::BestFit & bf = particle.getBestFit();
                    float lenwidthratio = bf.length / bf.width;
#endif // ? OLD_PARTICLE_WEDGE_SCHEME

                    if (outfmt == OUTPUTFMT_FORMAT_DG_UWA)
                    {
                        fprintf (do_outfp,
                                 "%d %d "
                                 "%.6f %.6f " // x, y
                                 "%d %d " // area, totalParticleNum
                                 "%d %d " // bbox width, bbox height
                                 "%.5f %.5f " // min, max
                                 "%.5f %.5f %.5f  " // minang,maxang,max/min
                                 "%.5f %.5f " // len, len_angle
                                 "%.5f %.5f %.5f\n", // wid, wid_ang, len/wid
                                 real_frame, camera_number,
                                 centroid_x, centroid_y,
                                 area, totalParticleNum,
                                 particle.maxx - particle.minx + 1,
                                 particle.maxy - particle.miny + 1,
                                 mindist, maxdist,
                                 mindistangle, maxdistangle, maxminratio,
                                 bf.length, bf.length_angle,
                                 bf.width, bf.width_angle, lenwidthratio);
                    }
                    else if (outfmt == OUTPUTFMT_FORMAT_AB_ODU)
                    {
#ifdef OLD_PARTICLE_WEDGE_SCHEME
                        fprintf (do_outfp,
                                 "%d,%d,%d,%.6f,%.6f,%d,%d,%.5f,%.5f,%.5f\n",
                                 real_frame, frameParticleNum, area,
                                 centroid_x, centroid_y, mindim, maxdim,
                                 mindimangle, maxdimangle, minmaxratio);
#else // ! OLD_PARTICLE_WEDGE_SCHEME
                        fprintf (do_outfp,
                                 "%d,%d,%d,"
                                 "%.6f,%.6f, %.5f,%.5f, " // x, y, min, max
                                 "%.5f,%.5f, %.5f,  " // minang,maxang,max/min
                                 "%.5f,%.5f, " // len, len_angle
                                 "%.5f,%.5f, %.5f\n", // wid, wid_ang, len/wid
                                 real_frame, frameParticleNum, area,
                                 centroid_x, centroid_y, mindist, maxdist,
                                 mindistangle, maxdistangle, maxminratio,
                                 bf.length, bf.length_angle,
                                 bf.width, bf.width_angle, lenwidthratio);
#endif // ? OLD_PARTICLE_WEDGE_SCHEME
                    }
                    else
                        fprintf (do_outfp,
                                 "uh oh, unknown fancy output format %d\n",
                                 outfmt);
                }
                else
                {
                    if (outfmt == OUTPUTFMT_FORMAT_NEW)
                    {
                        fprintf (do_outfp, "%d %d %.6f %.6f %d %d\n",
                                 real_frame, camera_number,
                                 centroid_x, centroid_y,
                                 area, totalParticleNum);
                    }
                    else
                    {
                        fprintf (do_outfp, "%d %d %.6f %.6f %d\n",
                                 totalParticleNum, area,
                                 centroid_x, centroid_y, real_frame);
                    }
                }
            }
        }
    }

    data->copyInfo(image);
    return 1;
}	                           

//////////////////////////////////////////////////////////////////////////////

#ifdef OLD_PARTICLE_WEDGE_SCHEME
inline void Particle_Old::Wedge::trim_to_edge ()
{
    for (int i = pixel_distance.size() - 1; i >= 0; --i)
    {
        if (pixel_distance [i] > max_white_distance)
        {
            // toss it; to save time we just mark it invalid instead of
            // deleting from the various vectors.
            pixel_distance [i] = -1;
            --pixel_count;
        }
    }
}
#endif

char ImageTool::directionLetters [] = "ESWN#?";

PixelOffset ImageTool::leftOffset [ImageTool::DIRECTION_COUNT] =
{
//     East         South        West         North
    {  0, -1 },  {  1,  0 },  {  0,  1 },  { -1,  0 }
};

// Interesting - aheadOffset [n] == leftOffset [(n + 1) % DIRECTION_COUNT]...
PixelOffset ImageTool::aheadOffset [ImageTool::DIRECTION_COUNT] =
{
//     East         South        West         North
    {  1,  0 },  {  0,  1 },  { -1,  0 },  {  0, -1 }
};

// If we're using 4-connectedness (current implementation), then the following
// would be treated as two independent particles; with 8-connectedness, this
// would be a single particle with some "holes" in it.

// . . . * * . . .
// . . * * * . . .
// . * * * . * . .
// . * * * . * * .
// * * * . * * * .
// * * . * * * * .
// . * . * * * . .
// . . . * . . . .


uint8_t ImageTool::autoOutline (uint32_t x, uint32_t y)
{
/*    my_particle.init (debug);
    PixelLocVec & outline = my_particle.outline;

    static int needbar = 1;

    if (needbar && (debug & 0xffff)) // anything
    {
        printf ("\n======================================="
                "=======================================\n\n");
        needbar = 0;
    }

    if ((debug & (0x04 << SHIFT_PAST_SHOW_FLAGS)) && ++needbar)
        printf ("autoOutline(%d,%d):", x, y);

    ADM_assert (goodPixel (x, y));

    uint32_t startingX = x;
    uint32_t startingY = y;

    TracingDirection direction = DIRECTION_COUNT; // deliberately invalid
    TracingDirection newDirection = DIRECTION_EAST;

    int8_t
        Lx, Ly,  // left
        Ax, Ay;  // ahead

#if 0
    uint32_t prevX;
    uint32_t prevY;
    TracingDirection startingDirection = direction;
#endif

    uint32_t pointCount = 0;
    uint32_t pointMax = my_w * my_h; // more than that and something is wrong

    do
    {
#if 0
        prevX = x;
        prevY = y;
#endif

        outline.push_back (PixelLoc (x, y));
        if ((debug & (0x08 << SHIFT_PAST_SHOW_FLAGS)) && ++needbar)
            printf (" %c(%d,%d)%c", directionLetters [direction],
                    x, y, directionLetters [newDirection]);

        if (++pointCount >= pointMax)
        {
            printf ("\nUh oh!  Too many points!! (%d) (@(%d,%d), dir %c)\n",
                    pointCount, x, y, directionLetters [direction]);
            showStuff (SHOW_INPUT);
            showStuff (SHOW_OUTLINE);
            break;
        }

        if (newDirection != direction)
        {
            direction = newDirection;
            Lx = leftOffset [direction].x;
            Ly = leftOffset [direction].y;
            Ax = aheadOffset [direction].x;
            Ay = aheadOffset [direction].y;
#define ARx (Ax + Rx)
#define ARy (Ay + Ry)
#define Rx  (-Lx)
#define Ry  (-Ly)
#define BRx (Bx + Rx)
#define BRy (By + Ry)
#define Bx  (-Ax)
#define By  (-Ay)
#define BLx (Bx + Lx)
#define BLy (By + Ly)
        }

        if (goodPixel (x + Lx, y + Ly))
        {
            // Turn left
            x += Lx;
            y += Ly;
            newDirection = TracingDirection ((direction + TURN_LEFT)
                                             % DIRECTION_COUNT);
        }
        else if (goodPixel (x + Ax, y + Ay))
        {
            // Continue in same direction
            x += Ax;
            y += Ay;
            // newDirection = direction;
        }
        else if (goodPixel (x + Rx, y + Ry))
        {
            // Turn right
            x += Rx;
            y += Ry;
            newDirection = TracingDirection ((direction + TURN_RIGHT)
                                             % DIRECTION_COUNT);
        }
#if 0
// if we turn this on, we probably ought to be looking at the AL pixel as
// well.  (And turn on the BR case, below?)

        else if (goodPixel (x + ARx, y + ARy))
        {
            // Slide down one pixel and continue
            do that!;
        }
#endif
        else if (goodPixel (x + Bx, y + By))
        {
            // Turn around (reverse direction)
            x += Bx;
            y += By;
            newDirection = TracingDirection ((direction + TURN_AROUND)
                                             % DIRECTION_COUNT);
        }
#if 0
        else if (goodPixel (x + BRx, y + BRy))
        {
            // Slide down one pixel and turn right
            do that (the slide)!;
            newDirection = TracingDirection ((direction + TURN_RIGHT)
                                             % DIRECTION_COUNT);
        }
#endif
        else
        {
            // Apparently there is nowhere to go.  This should happen only if
            // we're looking at an isolated pixel (a one-pixel particle).

            if (outline.size() > 1)
            {
                printf ("\nUh oh!  We're stuck, but it's not a lonely pixel!  "
                        "(@(%d,%d), dir %c)\n",
                        x, y, directionLetters [direction]);
                showStuff (SHOW_INPUT);
                showStuff (SHOW_OUTLINE);
                ADM_assert (outline.size() > 1);
            }

            break; // we'd fall out anyway, but this is faster & more explicit
        }

    } while (x != startingX || y != startingY);

    if ((debug & (0x04 << SHIFT_PAST_SHOW_FLAGS)) && ++needbar)
        printf (" done! %d points in all\n", outline.size());

    // We now do 5 things in one pass: 1. count the pixels in the particle
    // (thus computing the area, defined as the number of pixels in the
    // particle); 2. clear those pixels in the input image so that we don't
    // count the particle more than once; 3. compute the centroid (defined as
    // the point whose x is the average of all the x's in the particle, and
    // whose y is the average of all the y's in the particle); 4. if there is
    // an output image, draw the particle in the output, with the outline
    // highlighted; 5. compute the bounding box (rectangle that completely
    // encloses the particle).

    PixelLocVec::iterator plit = outline.begin();
    while (plit != outline.end())
    {
        uint32_t px = plit->x;
        uint32_t py = plit->y;

        ++plit;

        if (my_outImage)
        {
            outPixel (px, py) = 255;
#ifdef USE_COLOR_IN_OUTPUT
            outUPixel (px, py) = 0;
            outVPixel (px, py) = 0;
#endif
        }

        // We do the above (set up output pixels) first so we ensure that the
        // outline is highlighted even if the code below has already displayed
        // some of the pixels in the outline (due to direct or indirect
        // adjacency to other outline pixels).

        if (getPixel (px, py) == 0)
            continue;

        // If we're still here, then this pixel is part of the particle but
        // has not yet been counted or otherwise processed.  There may be
        // other horizontally adjacent pixels, as well, which we will also
        // process now, if they are present - this accomplishes "filling" the
        // particle outline (except that any pixels that were actually not
        // "on" in the particle will be ignored).

        // First, we process the pixel itself, and any pixels to the right of
        // it (if they are on).

        uint32_t startX = px;

        my_particle.addWhitePixel (px, py);
        getPixel (px, py) = 0; // erase to prevent being counted again

        while (goodPixel (++px, py))
        {
            my_particle.addWhitePixel (px, py);
            getPixel (px, py) = 0; // erase to prevent being counted again
            if (my_outImage)
                outPixel (px, py) = 128;
        }

        // Now, we process any pixels to the left of the original one (if they
        // are on).

        px = startX;
        while (goodPixel (--px, py))
        {
            my_particle.addWhitePixel (px, py);
            getPixel (px, py) = 0; // erase to prevent being counted again
            if (my_outImage)
                outPixel (px, py) = 128;
        }
    }

    if (my_particle.area < my_minArea || my_particle.area > my_maxArea)
    {
        if (my_outImage)
        {
            // We need to erase the particle we just drew. :-(

            plit = my_particle.white_pixels.begin();
            while (plit != my_particle.white_pixels.end())
            {
                outPixel (plit->x, plit->y) = 0;
                ++plit;
            }
        }

        return 0;
    }

    needbar = 1;

    my_particle.computeCentroid();

    if (debug & (0x0002 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
        printf ("    particle has %d pixels from (%d,%d) to (%d,%d) "
                "with centroid at (%.5f,%.5f)\n",
                my_particle.area, my_particle.minx, my_particle.miny,
                my_particle.maxx, my_particle.maxy,
                my_particle.centroidX, my_particle.centroidY);

    if (my_outImage)
    {
        uint32_t px = static_cast <uint32_t> (nearbyintf (my_particle.centroidX));
        uint32_t py = static_cast <uint32_t> (nearbyintf (my_particle.centroidY));
        if (validPixel (px, py))
            outPixel (px, py) = 255;
        else
            fprintf (stderr, "################# Uh oh, about to stomp "
                     "invalid centroid pixel (%d, %d)!!\n", px, py);
#ifdef USE_COLOR_IN_OUTPUT
        outUPixel (px, py) = 255;
        outVPixel (px, py) = 255;
#endif
    }

    if (debug & SHOW_INPUT)
        showStuff (SHOW_INPUT);

    if (debug & SHOW_OUTLINE)
        showStuff (SHOW_OUTLINE);

#ifdef OLD_PARTICLE_WEDGE_SCHEME

    if (my_flags & FLAG_DO_SHAPE)
    {
        typedef Particle::Wedge Wedge;
        typedef Particle::WedgeVec WedgeVec;
        typedef Particle::WedgePair WedgePair;
        typedef Particle::WedgePairVec WedgePairVec;

        // HERE: for DG/UWA format, we may only need the white pixels.  For
        // AB/ODU format, we need to find all the pixels which are no further
        // from the centroid than the furthest white pixel in the same wedge.
        // For now, for DG/UWA format, we will show both numbers, so we need
        // to measure the distance from the centroid for both formats.

        my_particle.init_wedges();

        // HERE: we don't need to sort the white_pixels vector unless we're
        // counting non-white pixels in the shape stuff.

        PixelLocVec & white_pixels = my_particle.white_pixels;
        sort (white_pixels.begin(), white_pixels.end(), PixelLoc::Compare());

        WedgeVec & wedges = my_particle.wedges;

        uint16_t minx = my_particle.minx;
        uint16_t miny = my_particle.miny;
        uint16_t maxx = my_particle.maxx;
        uint16_t maxy = my_particle.maxy;

        const float pi_plus_half_wedge_arc
            = M_PI + my_particle.half_wedge_arc;
        const float wedge_arc = my_particle.wedge_arc;
        const uint32_t num_wedges = my_particle.num_wedges;

        for (uint16_t py = miny; py <= maxy; py++)
        {
            const float dy = py - my_particle.centroidY;
            const float dy_squared = dy * dy;

            for (uint16_t px = minx; px <= maxx; px++)
            {
                float dx = px - my_particle.centroidX;
                float angle = atan2f (dy, dx);
                uint32_t wedgeIndex
                    = int ((angle + pi_plus_half_wedge_arc) / wedge_arc);
                wedgeIndex %= num_wedges; // just in case

                // uint8_t value = getPixel (px, py);
                // oops, can't use that - we erased it already. :-(
                bool value
                    = binary_search (white_pixels.begin(), white_pixels.end(),
                                     PixelLoc (px, py), PixelLoc::Compare());
                float distance = sqrtf (dx * dx + dy_squared);
                wedges [wedgeIndex].add_pixel (px, py, value, distance);
                                               
                printf ("%d @ (%d,%d) %.5f -> %d @ %.5f\n",
                        value, px, py, distance,
                        wedgeIndex, wedges[wedgeIndex].center_angle);
            }
        }

        // See note in init_wedges() about what we're doing with the last
        // wedge here.

        wedges [0] += wedges [num_wedges];
        wedges.pop_back();

        // Now, in each wedge, toss any non-white pixels that are further out
        // than the furthest white pixel in that wedge.

        for (WedgeVec::iterator wit = wedges.begin();
             wit != wedges.end();
             ++wit)
        {
            printf ("wedge @ %.5f: max white distance = %.5f\n",
                    wit->center_angle, wit->max_white_distance);
            wit->trim_to_edge();
        }

        // Next, we combine opposite wedges so that we can report the
        // dimension along a line through the centroid across the whole
        // particle (not just from the centroid to the particle boundary).

        WedgePairVec & wedge_pairs = my_particle.wedge_pairs;
        int num_lines = my_particle.num_wedges / 2;
        for (int widx = 0; widx < num_lines; ++widx)
        {
            const Wedge & neg_wedge = wedges [widx];
            const Wedge & pos_wedge = wedges [widx + num_lines];

            float angle = pos_wedge.center_angle;
            printf ("wedge %d at %.5f = %d of %d  "
                    "wedge %d at %.5f = %d of %d\n",
                    widx, neg_wedge.center_angle, neg_wedge.pixel_count,
                    neg_wedge.pixel_distance.size(),
                    widx + num_lines, angle, pos_wedge.pixel_count,
                    pos_wedge.pixel_distance.size());
            uint32_t pixel_count
                = pos_wedge.pixel_count + neg_wedge.pixel_count;
            wedge_pairs.push_back (WedgePair (angle, pixel_count));
        }

        // Now we (effectively) sort the wedges by the number of pixels in
        // them.

        sort (wedge_pairs.begin(), wedge_pairs.end(), WedgePair::Less());

        if (debug & SHOW_WEDGE_PIXELS)
            showStuff (SHOW_WEDGE_PIXELS);
    }

#else // ! OLD_PARTICLE_WEDGE_SCHEME

    if (my_flags & FLAG_DO_SHAPE)
    {
        typedef Particle::Wedge Wedge;
        typedef Particle::WedgeVec WedgeVec;
        typedef Particle::WedgePair WedgePair;
        typedef Particle::WedgePairVec WedgePairVec;

        my_particle.init_wedges();

        PixelLocVec & white_pixels = my_particle.white_pixels;
        WedgeVec & wedges = my_particle.wedges;

        uint16_t minx = my_particle.minx;
        uint16_t miny = my_particle.miny;
        uint16_t maxx = my_particle.maxx;
        uint16_t maxy = my_particle.maxy;

        const float wedge_interval = my_particle.wedge_interval;
        const uint32_t num_wedges = my_particle.num_wedges;

        for (PixelLocVec::const_iterator wpit = white_pixels.begin();
             wpit != white_pixels.end();
             ++wpit)
        {
            const uint16_t px = wpit->x;
            const uint16_t py = wpit->y;
            const float dy = py - my_particle.centroidY;
            const float dx = px - my_particle.centroidX;
            float angle = atan2f (dy, dx);
            uint32_t wedgeIndex = int ((angle + M_PI) / wedge_interval);
            float distance = sqrtf (dx * dx + dy * dy);

            while (1)
            {
                wedgeIndex %= num_wedges + 1;

                Wedge & wedge = wedges [wedgeIndex];

                if (angle < wedge.min_angle || angle > wedge.max_angle)
                {
                    if (debug & (0x0020 << SHIFT_PAST_SHOW_FLAGS))
                        printf ("(%d,%d) = %.5f @ %.5f out of range for "
                                "%d @ %.5f (%.5f to %.5f)\n",
                                px, py, distance, angle,
                                wedgeIndex, wedge.center_angle,
                                wedge.min_angle, wedge.max_angle);
                    break;
                }

                wedge.add_pixel (px, py, distance, angle);

                if (debug & (0x0010 << SHIFT_PAST_SHOW_FLAGS))
                    printf ("(%d,%d) = %.5f @ %.5f -> %d @ %.5f (%.5f)\n",
                            px, py, distance, angle, wedgeIndex,
                            wedge.angle_for_mwd, wedge.center_angle);
                ++wedgeIndex;
            }
        }

        // See note in init_wedges() about what we're doing with the last
        // wedge here.

        wedges [0] += wedges [num_wedges];
        wedges.pop_back();

        // Next, we combine opposite wedges so that we can report the
        // dimension along a line through the centroid across the whole
        // particle (not just from the centroid to the particle boundary).

        WedgePairVec & wedge_pairs = my_particle.wedge_pairs;
        int num_lines = my_particle.num_wedges / 2;
        for (int widx = 0; widx < num_lines; ++widx)
        {
            const Wedge & neg_wedge = wedges [widx];
            const Wedge & pos_wedge = wedges [widx + num_lines];

            wedge_pairs.push_back (WedgePair (neg_wedge, pos_wedge));
            const WedgePair & wp = wedge_pairs.end()[-1];
            if (debug & (0x0100 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
                printf ("%c: wedge %d = %8.5f @ %.5f (%.5f), + "
                        "wedge %d = %8.5f @ %.5f (%.5f), --> %9.5f @ %.5f\n",
                        widx + 'a', widx, neg_wedge.max_white_distance,
                        neg_wedge.angle_for_mwd, neg_wedge.center_angle,
                        widx + num_lines, pos_wedge.max_white_distance,
                        pos_wedge.angle_for_mwd, pos_wedge.center_angle,
                        wp.total_distance, wp.angle);
        }

        // Now, we find the pair of perpendicular wedges with the greatest
        // ratio of length to width, where length and width are arbitrary
        // designations (but length will never be less than width).

        float best_ratio = -1;
        int best_ratio_index = -1;
        for (int wpidx = 0; wpidx < num_lines; ++wpidx)
        {
            const WedgePair & wp = wedge_pairs [wpidx];
            // Find the perpendicular wedge.
            int pwpidx = (wpidx + (num_lines / 2)) % num_lines;
            const WedgePair & pwp = wedge_pairs [pwpidx];
            float ratio = wp.total_distance / pwp.total_distance;
            if (debug & (0x0200 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
                printf ("%c: wedgepair %d = %9.5f @ %.5f / "
                        "wedgepair %d = %9.5f @ %.5f = %.5f\n",
                        wpidx + 'a', wpidx, wp.total_distance, wp.angle,
                        pwpidx, pwp.total_distance, pwp.angle, ratio);
            if (ratio > best_ratio)
            {
                best_ratio = ratio;
                best_ratio_index = wpidx;
            }
        }

        Particle::BestFit & best_fit = my_particle.best_fit;
        const WedgePair & wp = wedge_pairs [best_ratio_index];
        best_fit.length = wp.total_distance;
        best_fit.length_angle = wp.angle;
        int best_ratio_width_index
            = (best_ratio_index + (num_lines / 2)) % num_lines;
        const WedgePair & pwp = wedge_pairs [best_ratio_width_index];
        best_fit.width = pwp.total_distance;
        best_fit.width_angle = pwp.angle;

        if (debug & (0x0200 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
            printf ("\nbest fit: length = wedgepair %d (%.5f @ %.5f) X "
                    "wedgepair %d (%.5f @ %.5f); len/wid = %.5f\n\n",
                    best_ratio_index, wp.total_distance, wp.angle,
                    best_ratio_width_index, pwp.total_distance, pwp.angle,
                    best_ratio);

        // Finally, we (effectively) sort the wedges by the number of pixels
        // in them.

        sort (wedge_pairs.begin(), wedge_pairs.end(), WedgePair::Less());

        if (debug & SHOW_WEDGE_PIXELS)
            showStuff (SHOW_WEDGE_PIXELS);
    }

#endif // ? OLD_PARTICLE_WEDGE_SCHEME
*/
    return 1;
}

const PixelLoc PixelLoc::INVALID (0xffff, 0xffff);

#ifdef OLD_PARTICLE_WEDGE_SCHEME

void Particle_Old::init_wedges ()
{
    // num_wedges = (maxx - minx + 1 + maxy - miny + 1) * 2;
    //    The above was too many - not enough pixels in any one wedge, and
    //    minimum was always zero.
    // num_wedges = maxx - minx + 1 + maxy - miny + 1;
    //    The above was still too many.
    num_wedges = min (int (area / 2), maxx - minx + 1 + maxy - miny + 1);

    num_wedges = (num_wedges + 3) & ~3; // round up to multiple of 4
    wedge_arc = 2.0 * M_PI / num_wedges;
    half_wedge_arc = wedge_arc / 2;
    printf ("init_wedges(): area = %d; %d wedges of arc %.5f\n",
            area, num_wedges, wedge_arc);

    // We are going to set up one extra wedge, because out on the negative X
    // axis, there is a discontinuity: angles will be around +pi or -pi,
    // depending on whether they are just above or below the negative X axis.
    // So with the extra wedge, we have two exactly overlapping wedges (the
    // first and last ones), with the first one catching all the ~-pi values
    // and the last one catching the ~+pi values.  Later, we'll dump the last
    // one into the first one before we process the wedges themselves.

    wedges.reserve (num_wedges + 1);
    double angle = -M_PI;
    uint32_t wedge_index = 0;
    while (wedge_index <= num_wedges)
    {
        wedges.push_back (Wedge (angle, half_wedge_arc));
        angle += wedge_arc;
        ++wedge_index;
    }
}

#else // ! OLD_PARTICLE_WEDGE_SCHEME

void Particle::init_wedges ()
{
    // We've tried various other things, but they didn't work out well.
    // Now we have a fixed number of wedges, and each wedge overlaps with 50%
    // of each of its neighbors (e.g., 16 wedges, each one 1/8 of the circle).

    num_wedges = 16;
    wedge_interval = 2.0 * M_PI / num_wedges;
    wedge_arc = wedge_interval * 2;
    half_wedge_arc = wedge_arc / 2;
    if (debug & (0x40 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
        printf ("init_wedges(): %d wedges of arc %.5f, centered every %.5f\n",
                num_wedges, wedge_arc, wedge_interval);

    // We are going to set up one extra wedge, because out on the negative X
    // axis, there is a discontinuity: angles will be around +pi or -pi,
    // depending on whether they are just above or below the negative X axis.
    // So with the extra wedge, we have two exactly overlapping wedges (the
    // first and last ones), with the first one catching all the ~-pi values
    // and the last one catching the ~+pi values.  Later, we'll dump the last
    // one into the first one before we process the wedges themselves.

    wedges.reserve (num_wedges + 1);
    double angle = -M_PI;
    uint32_t wedge_index = 0;
    while (wedge_index <= num_wedges)
    {
        if (debug & (0x80 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
            printf ("    wedge %2d: %.5f (%.5f to %.5f)\n",
                    wedge_index, angle, angle - half_wedge_arc,
                    angle + half_wedge_arc);
        wedges.push_back (Wedge (angle, half_wedge_arc));
        angle += wedge_interval;
        ++wedge_index;
    }
}

#endif // ? OLD_PARTICLE_WEDGE_SCHEME

static inline
char & pixel_char (vector <string> & grid, int16_t xbase, int16_t ybase,
                   float cX, float cY, float dX, float dY)
{
    uint16_t x = static_cast <uint16_t> (nearbyintf (cX + dX));
    uint16_t y = static_cast <uint16_t> (nearbyintf (cY + dY));
    return grid [y - ybase][x - xbase];
}

static inline
char & pixel_char_from_angle (vector <string> & grid, int16_t xbase, int16_t ybase,
                              float cX, float cY, float distance, float angle)
{
    float dX = distance * cosf (angle);
    float dY = distance * sinf (angle);
    return pixel_char (grid, xbase, ybase, cX, cY, dX, dY);
}

void ImageTool::showStuff (ShowStuff what) const
{
    const PixelLocVec & outline = my_particle.outline;

    const int margin = 5;

    uint16_t bbminx = my_particle.minx;
    uint16_t bbminy = my_particle.miny;
    uint16_t bbmaxx = my_particle.maxx;
    uint16_t bbmaxy = my_particle.maxy;

    if (bbminx > bbmaxx)
    {
        PixelLocVec::const_iterator plit = outline.begin();
        while (plit != outline.end())
        {
            uint32_t px = plit->x;
            uint32_t py = plit->y;

            if (px < bbminx)
                bbminx = px;
            if (px > bbmaxx)
                bbmaxx = px;
            if (py < bbminy)
                bbminy = py;
            if (py > bbmaxy)
                bbmaxy = py;

            ++plit;
        }
    }

    uint16_t minx = max (int16_t (bbminx) - margin, 0);
    uint16_t miny = max (int16_t (bbminy) - margin, 0);
    uint16_t maxx = min (bbmaxx + margin, int16_t (my_w) - 1);
    uint16_t maxy = min (bbmaxy + margin, int16_t (my_h) - 1);

    switch (what)
    {
    case SHOW_INPUT:
        printf ("Input (after previous particle(s) erased):\n\n");
        break;

    case SHOW_OUTLINE:
        printf ("Outline (%d points):\n\n", outline.size());
        break;

#ifdef OLD_PARTICLE_WEDGE_SCHEME
    case SHOW_WEDGE_PIXELS:
        printf ("Wedge pair assignments (%d wedges):\n\n",
                my_particle.num_wedges);
        break;
#else
    case SHOW_WEDGE_PIXELS:
        printf ("Wedge distances (%d wedges, %.5f arc at %.5f interval):\n\n",
                my_particle.num_wedges, my_particle.wedge_arc,
                my_particle.wedge_interval);
        break;
#endif

    default:
        break;
    }

    uint16_t dimx = maxx - minx + 1;
    uint16_t dimy = maxy - miny + 1;
    int16_t xbase = minx - 1;
    int16_t ybase = miny;

    if (debug & (0x0400 << ImageTool::SHIFT_PAST_SHOW_FLAGS))
        printf ("%dx%d (%d,%d - %d,%d; %d,%d - %d,%d; base = %d,%d)\n",
                dimx, dimy, bbminx, bbminy, bbmaxx, bbmaxy,
                minx, miny, maxx, maxy, xbase, ybase);

    string ruler (dimx + 2, '-');
    ruler [0] = '+';
    ruler [dimx + 1] = '+';
    printf ("  x|");
    const int x_ruler_spacing = 5;
    unsigned xtick = minx + x_ruler_spacing - (minx % x_ruler_spacing);
    unsigned field_width = xtick - minx + 2;
    while (xtick < maxx)
    {
        printf ("%*d", field_width, xtick);
        ruler [xtick - xbase] = '+';
        xtick += x_ruler_spacing;
        field_width = x_ruler_spacing;
    }
    ruler [bbminx - xbase] = '|';
    ruler [bbmaxx - xbase] = '|';
    printf ("\ny: %s\n", ruler.c_str());

    string blankLine (dimx + 2, '.');
    blankLine [0] = '|';
    blankLine [dimx + 1] = '|';
    vector <string> grid (dimy, blankLine);
    grid [bbminy - ybase][0] = '=';
    grid [bbminy - ybase][dimx + 1] = '=';
    grid [bbmaxy - ybase][0] = '=';
    grid [bbmaxy - ybase][dimx + 1] = '=';

    switch (what)
    {
    case SHOW_INPUT:
    case SHOW_OUTLINE:
    {
        for (uint32_t py = miny; py <= maxy; py++)
        {
            string & gridline = grid [py - ybase];
            for (uint32_t px = minx; px <= maxx; px++)
                if (goodPixel (px, py))
                    gridline [px - xbase] = 'x';
        }
    }

    // case SHOW_OUTLINE:
    {
        PixelLocVec::const_iterator plit = my_particle.white_pixels.begin();
        while (plit != my_particle.white_pixels.end())
        {
            grid [plit->y - ybase][plit->x - xbase] = '*';
            ++plit;
        }

        if (what == SHOW_INPUT)
            break;

        plit = outline.begin();
        while (plit != outline.end())
        {
            uint32_t px = plit->x;
            uint32_t py = plit->y;

            char & ch = grid [py - ybase][px - xbase];
            // printf ("(%d,%d)%c", px, py, ch);
            if (ch == '*')
                ch = '1';
            else if (ch == '9')
                ch = '+';
            else if (ch == '.')
                ch = '1';
            else if (ch != '+')
                ++ch;
            // printf ("->%c  ", ch);

            ++plit;
        }

        uint32_t px = static_cast <uint32_t> (nearbyintf (my_particle.centroidX));
        uint32_t py = static_cast <uint32_t> (nearbyintf (my_particle.centroidY));
        char & ch = grid [py - ybase][px - xbase];
        if (ch == 'x')
            ch = 'X';
        else if (ch == '.')
            ch = 'c';
        else
            ch = 'C';
        break;
    }

#ifdef OLD_PARTICLE_WEDGE_SCHEME
    case SHOW_WEDGE_PIXELS:
    {
        const Particle::WedgeVec & wedges = my_particle.wedges;
        int num_lines = my_particle.num_wedges / 2;
        for (int widx = 0; widx < num_lines; ++widx)
        {
            char ch = 'a' + (widx % 26);
            for (int incr = 0; incr <= num_lines; incr += num_lines)
            {
                const Particle::Wedge & wedge = wedges [widx + incr];
                PixelLocVec::const_iterator plit = wedge.pixel_locs.begin();
                vector <float>::const_iterator fit
                    = wedge.pixel_distance.begin();
                while (plit != wedge.pixel_locs.end())
                {
                    if (*fit++ > 0)
                        grid [plit->y - ybase][plit->x - xbase] = ch;
                    ++plit;
                }
            }
        }
        uint32_t px = static_cast <uint32_t> (nearbyintf (my_particle.centroidX));
        uint32_t py = static_cast <uint32_t> (nearbyintf (my_particle.centroidY));
        char & ch = grid [py - ybase][px - xbase];
        if (ch == '.')
            ch = '*';
        else
            ch = toupper (ch);
        break;
    }
#else // ! OLD_PARTICLE_WEDGE_SCHEME
    case SHOW_WEDGE_PIXELS:
    {
        // For each wedge, we mark the max distance for it with a lowercase
        // letter at the point along the angle for that wedge that is the max
        // distance from the centroid.
        // 
        // We mark the best-fit points with an L at points half the "length"
        // from the centroid along the angle for the length and a W at points
        // half the "width" from the centroid along the angle for the width.

        const Particle::WedgeVec & wedges = my_particle.wedges;
        const float centroidX = my_particle.centroidX;
        const float centroidY = my_particle.centroidY;
        int num_lines = my_particle.num_wedges / 2;
        for (int widx = 0; widx < num_lines; ++widx)
        {
            char newch = 'a' + (widx % 26);
            for (int incr = 0; incr <= num_lines; incr += num_lines)
            {
                const Particle::Wedge & wedge = wedges [widx + incr];
                char & ch = pixel_char_from_angle (grid, xbase, ybase,
                                                   centroidX, centroidY,
                                                   wedge.max_white_distance,
                                                   wedge.angle_for_mwd);
                // printf ("(%d,%d) = '%c' (%.5f @ %.5f)\n",
                //         x, y, newch, wedge.max_white_distance,
                //         wedge.angle_for_mwd);
                ch = newch;
            }
        }
        const Particle::BestFit & bf = my_particle.best_fit;
        pixel_char_from_angle (grid, xbase, ybase, centroidX, centroidY,
                               bf.length / 2, bf.length_angle)
            = 'L';
        pixel_char_from_angle (grid, xbase, ybase, centroidX, centroidY,
                               bf.length / 2, bf.length_angle - M_PI)
            = 'L';
        pixel_char_from_angle (grid, xbase, ybase, centroidX, centroidY,
                               bf.width / 2, bf.width_angle)
            = 'W';
        pixel_char_from_angle (grid, xbase, ybase, centroidX, centroidY,
                               bf.width / 2, bf.width_angle - M_PI)
            = 'W';
        char & ch = pixel_char (grid, xbase, ybase,
                                centroidX, centroidY, 0, 0);
        if (ch == '.')
            ch = '*';
        else
            ch = '@';
        break;
    }
#endif // ? OLD_PARTICLE_WEDGE_SCHEME

    default:
        break;
    }

    for (unsigned i = miny; i <= maxy; ++i)
        printf ("%3d%s\n", i, grid [i - ybase].c_str());

    printf ("   %s\n\n", ruler.c_str());
}

// Debug output should be able to show:

// input: contents of entire bounding box plus 5-pixel margin on all sides
// (with box boundaries marked somehow)

// outline (current showStuff()) but with margin as above and showing all
// pixel states (on/off) (on = *, C for centroid; . = off)

// all pixels considered part of the particle, with wedge assignments if using
// a shape-reporting format, and showing centroid.  each pixel is shown as
// (wedge # % 26) + 'a', and (closest pixel to) centroid is in uppercase, or
// a * if not actually a white pixel.  (use same letter for opposite wedges.)

// list of wedges in angle order, showing their angles (center & bounds),
// on/off/included pixel counts, and optionally list of all pixels in them

// should show the angles to the bounding box corners, for comparison

// list of wedges in percentile order, showing their percentiles, angles, etc.

// debug option to draw line through the max angle - better, draw bounding box
// (+1 on all sides), with lines from bounding box edges outward (20 pixels
// for len, 10 for width)

//   x|260 264 268 272 276
// y: +-+-|-+---+---+-|-+-+
// 120|...................|
// 121|...................|
// 122|...................|
// 123=.....11111.........=
// 124|...111***1111......|
// 125|....111**C**111....|
// 126|......111**11111...|
// 127=........1111.......=
// 128|...................|
// 129|...................|
// 130|...................|
//    +---|-----------|---+
