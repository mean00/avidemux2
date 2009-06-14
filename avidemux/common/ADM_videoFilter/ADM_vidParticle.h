/***************************************************************************
                          ADM_vidParticle.h  -  detect particles (groups of pixels)
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
 
#ifndef __PARTICLE__
#define __PARTICLE__   

#include <string>
#include <algorithm>
#include <math.h>


struct PARTICLE_PARAM
{
    uint32_t min_area;
    uint32_t max_area;
    uint32_t left_crop;
    uint32_t right_crop;
    uint32_t top_crop;
    uint32_t bottom_crop;
    uint32_t output_format;
    std::string output_file;
    uint32_t camera_number;
    uint32_t min_dim_pctile;
    uint32_t max_dim_pctile;
    uint32_t debug;
};

// Alas, because offsetof() is only supposed to work on POD (plain old data)
// structs, and our PARTICLE_PARAM includes a std::string (which has a
// constructor, and which causes PARTICLE_PARAM to therefore have an implicit
// constructor), we need to define our own offsetof() to use for the dialog
// menus.  See
// http://www.cplusplus.com/reference/clibrary/cstddef/offsetof.html for more
// on offsetof().

#define my_offsetof(_type, _memb) (size_t (&(((_type *)1)->_memb)) - 1)

class ADMVideoParticle : public AVDMGenericVideoStream
{

 protected:
    	
     PARTICLE_PARAM *  _param;

     static FILE * outfp; // ugly, but hard to avoid
     static uint32_t last_frame_written; // ditto

 public:
 		
     enum OutputFmt
     {
         OUTPUTFMT_INVALID = 0,

         OUTPUTFMT_FORMAT_NEW,
         OUTPUTFMT_FORMAT_DG_UWA,
         OUTPUTFMT_FORMAT_AB_ODU,
         OUTPUTFMT_FORMAT_OLD,

         OUTPUTFMT_COUNT
     };

     ADMVideoParticle (AVDMGenericVideoStream *in, CONFcouple *setup);

     ~ADMVideoParticle();

     virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t *len,
                                            ADMImage *data,uint32_t *flags);

     virtual uint8_t configure (AVDMGenericVideoStream *instream);
     virtual char * printConf (void);
     virtual uint8_t getCoupledConf (CONFcouple **couples);
							
     static uint8_t doParticle (ADMImage * image, ADMImage * data,
                                AVDMGenericVideoStream * in,
                                uint32_t real_frame,
                                FILE * do_outfp,
                                PARTICLE_PARAM * param,
                                uint32_t width, uint32_t height);
};

//////////////////////////////////////////////////////////////////////////////

#include <list>
//#include <vector>
//#include <iostream>
#include <iterator>

class PixelLoc
{
public:
    uint16_t x;
    uint16_t y;

    PixelLoc ()
        : x (0),
          y (0)
    {
    }

    PixelLoc (uint32_t a_x, uint32_t a_y)
        : x (a_x),
          y (a_y)
    {
    }

    // Use this to sort by y's, then (for PixelLoc's with the same y) by x's.

    class Compare
    {
    public:
        int operator () (const PixelLoc & p1, const PixelLoc & p2) const
        {
            return ((p1.y == p2.y) ? (p1.x < p2.x) : (p1.y < p2.y));
        }
    };

    bool operator == (const PixelLoc & rhs)
    {
        return (x == rhs.x && y == rhs.y);
    }

    static const PixelLoc INVALID;
};

//typedef std::vector <PixelLoc> PixelLocVec;
/*
inline std::ostream &
operator << (std::ostream & s, const PixelLoc & pl)
{
    return s << "(" << pl.x << "," << pl.y << ")";
}

inline std::ostream &
operator << (std::ostream & s, const PixelLocVec & theList)
{
    s << "[ ";
    std::copy (theList.begin(), theList.end(),
               std::ostream_iterator <PixelLoc> (s, " "));
    s << "]";
    return s;
}
*/
class ImageTool;

#ifdef OLD_PARTICLE_WEDGE_SCHEME

class Particle_Old
{
    struct Wedge
    {
        // HERE: if we're only counting white pixels, then we actually don't
        // need any of the three vectors except for debugging output - we
        // could just keep a count of pixels and that would be enough.

        uint32_t pixel_count;
        std::vector <float> pixel_distance;
        PixelLocVec pixel_locs; // used only for debug output, else empty
        std::vector <char> is_white; // faster than vector<bool>
        float max_white_distance;
        float min_angle;
        float center_angle;
        float max_angle;
        float percentile;

    private: // probably we don't need the default ctor - let's see...
        Wedge ()
            : pixel_count (0),
              max_white_distance (-1),
              min_angle (-10 * M_PI),
              center_angle (-10 * M_PI),
              max_angle (-10 * M_PI),
              percentile (-1)
        {
        }
    public:

        Wedge (float angle, float half_arc)
            : pixel_count (0),
              max_white_distance (-1),
              min_angle (angle - half_arc),
              center_angle (angle),
              max_angle (angle + half_arc),
              percentile (-1)
        {
        }

        void add_pixel (uint16_t x, uint16_t y)
        {
            ++pixel_count;
            pixel_locs.push_back (PixelLoc (x, y));
        }

        void add_pixel (uint16_t x, uint16_t y, bool is_it_white, float distance)
        {
            ++pixel_count;
            is_white.push_back (is_it_white);
            if (is_it_white && max_white_distance < distance)
                max_white_distance = distance;
            pixel_distance.push_back (distance);
            pixel_locs.push_back (PixelLoc (x, y));
        }

        Wedge & operator += (const Wedge & rhs)
        {
            pixel_count += rhs.pixel_count;
            pixel_distance.insert (pixel_distance.end(),
                                   rhs.pixel_distance.begin(),
                                   rhs.pixel_distance.end());
            pixel_locs.insert (pixel_locs.end(),
                               rhs.pixel_locs.begin(),
                               rhs.pixel_locs.end());
            is_white.insert (is_white.end(),
                             rhs.is_white.begin(),
                             rhs.is_white.end());
            if (rhs.max_white_distance > max_white_distance)
                max_white_distance = rhs.max_white_distance;

            return *this;
        }

        class Less
        {
        public:
            bool operator () (const Wedge & lhs, const Wedge & rhs) const
            {
                return (lhs.pixel_count < rhs.pixel_count);
            }
        };

        void trim_to_edge ();
    };

    //typedef std::vector <Wedge> WedgeVec;

    struct WedgePair
    {
        float angle;
        uint32_t pixel_count;

        WedgePair (float angle, uint32_t pixel_count)
            : angle (angle),
              pixel_count (pixel_count)
        {
        }

        class Less
        {
        public:
            bool operator () (const WedgePair & lhs,
                              const WedgePair & rhs) const
            {
                return (lhs.pixel_count < rhs.pixel_count);
            }
        };
    };

    typedef std::vector <WedgePair> WedgePairVec;

public:
    uint16_t minx, maxx; // bounding box x
    uint16_t miny, maxy; // bounding box y
    uint32_t area;
    float centroidX, centroidY;

private:
    uint32_t sumOfXs;
    uint32_t sumOfYs;
    uint32_t num_wedges;
    // doubles used to minimize error accumulation when generating wedges
    double wedge_arc;
    double half_wedge_arc;

    PixelLocVec outline;
    PixelLocVec white_pixels;
    //WedgeVec wedges;
    WedgePairVec wedge_pairs;

public:
    Particle ()
    {
        init();
    }

private:
    void init ()
    {
        minx = 0xffff;
        maxx = 0;
        miny = 0xffff;
        maxy = 0;
        area = 0;
        centroidX = -1;
        centroidY = -1;
        sumOfXs = 0;
        sumOfYs = 0;
        num_wedges = 0;
        wedge_arc = -1;
        half_wedge_arc = -1;
        outline.clear();
        white_pixels.clear();
        wedges.clear();
        wedge_pairs.clear();
    }

    void init_wedges ();

    void addWhitePixel (uint16_t x, uint16_t y)
    {
        ++area;
        sumOfXs += x;
        if (x < minx)
            minx = x;
        if (x > maxx)
            maxx = x;
        sumOfYs += y;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
        white_pixels.push_back (PixelLoc (x, y));
    }

    void computeCentroid ()
    {
        if (!area)
        {
            fprintf (stderr, "Can't compute centroid of zero-pixel particle!\n");
            centroidX = 0;
            centroidY = 0;
            return;
        }
        centroidX = float (sumOfXs) / area;
        centroidY = float (sumOfYs) / area;
    }

public:
    const WedgePair & getMinDim () const
    {
        WedgePairVec::const_iterator wit = wedge_pairs.begin();
        while (!wit->pixel_count)
            ++wit;
        return *wit;
    }

    const WedgePair & getMaxDim () const
    {
        WedgePairVec::const_iterator wit = wedge_pairs.end();
        return *--wit;
    }

    uint32_t getMinDimCount () const
    {
        return getMinDim().pixel_count;
    }

    uint32_t getMaxDimCount () const
    {
        return getMaxDim().pixel_count;
    }

    float getMinDimAngle () const
    {
        return getMinDim().angle;
    }

    float getMaxDimAngle () const
    {
        return getMaxDim().angle;
    }

    friend class ImageTool;
};

#else // ! OLD_PARTICLE_WEDGE_SCHEME

class Particle
{
    struct Wedge
    {
        float max_white_distance;
        float angle_for_mwd;
        float min_angle;
        float center_angle;
        float max_angle;
        float percentile;

        Wedge (float angle, float half_arc)
            : max_white_distance (.5), // minimum width is 1
              angle_for_mwd (angle),
              min_angle (angle - half_arc),
              center_angle (angle),
              max_angle (angle + half_arc),
              percentile (-1)
        {
        }

        void add_pixel (uint16_t x, uint16_t y, float distance, float angle)
        {
            if (max_white_distance < distance)
            {
                max_white_distance = distance;
                angle_for_mwd = angle;
            }
        }

        Wedge & operator += (const Wedge & rhs)
        {
            if (rhs.max_white_distance > max_white_distance)
            {
                max_white_distance = rhs.max_white_distance;
                angle_for_mwd = rhs.angle_for_mwd;
            }

            return *this;
        }

        class Less
        {
        public:
            bool operator () (const Wedge & lhs, const Wedge & rhs) const
            {
                return (lhs.max_white_distance < rhs.max_white_distance);
            }
        };
    };

    //typedef std::vector <Wedge> WedgeVec;

    struct WedgePair
    {
        float angle;
        float total_distance;

        WedgePair (const Wedge & w1, const Wedge & w2)
            : total_distance (w1.max_white_distance + w2.max_white_distance)
        {
            float a1 = w1.angle_for_mwd;
            float a2 = w2.angle_for_mwd;
            if (a1 > a2)
                std::swap (a1, a2);
            a1 += M_PI;
            float avg = (a1 + a2) / 2;
            if (avg < 0)
                avg += M_PI;
            angle = avg;
        }

        class Less
        {
        public:
            bool operator () (const WedgePair & lhs,
                              const WedgePair & rhs) const
            {
                return (lhs.total_distance < rhs.total_distance);
            }
        };
    };

    //typedef std::vector <WedgePair> WedgePairVec;

public:
    uint16_t minx, maxx; // bounding box x
    uint16_t miny, maxy; // bounding box y
    uint32_t area;
    float centroidX, centroidY;

    struct BestFit
    {
        float length;
        float length_angle;
        float width;
        float width_angle;

        BestFit ()
            : length (-1),
              length_angle (0),
              width (-1),
              width_angle (0)
        {
        }
    };

private:
    uint32_t debug;
    uint32_t sumOfXs;
    uint32_t sumOfYs;
    uint32_t num_wedges;
    // doubles used to minimize error accumulation when generating wedges
    double wedge_interval; // 2*PI / num_wedges
    double wedge_arc;      // is > wedge_interval if overlapped with neighbors
    double half_wedge_arc;
    BestFit best_fit;

    //PixelLocVec outline;
    //PixelLocVec white_pixels;
    //WedgeVec wedges;
    //WedgePairVec wedge_pairs;

public:
    Particle ()
    {
        init (debug);
    }

private:
    void init (uint32_t a_debug)
    {
        minx = 0xffff;
        maxx = 0;
        miny = 0xffff;
        maxy = 0;
        area = 0;
        centroidX = -1;
        centroidY = -1;
        sumOfXs = 0;
        sumOfYs = 0;
        num_wedges = 0;
        wedge_arc = -1;
        wedge_interval = -1;
        half_wedge_arc = -1;
        best_fit = BestFit();
        //outline.clear();
        //white_pixels.clear();
        //wedges.clear();
        //wedge_pairs.clear();
        debug = a_debug;
    }

    void init_wedges ();

    void addWhitePixel (uint16_t x, uint16_t y)
    {
        ++area;
        sumOfXs += x;
        if (x < minx)
            minx = x;
        if (x > maxx)
            maxx = x;
        sumOfYs += y;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
        //white_pixels.push_back (PixelLoc (x, y));
    }

    void computeCentroid ()
    {
        if (!area)
        {
            fprintf (stderr, "Can't compute centroid of zero-pixel particle!\n");
            centroidX = 0;
            centroidY = 0;
            return;
        }
        centroidX = float (sumOfXs) / area;
        centroidY = float (sumOfYs) / area;
    }

public:
    const BestFit & getBestFit () const
    {
        return best_fit;
    }

    const WedgePair & getMinDist () const
    {
/*        WedgePairVec::const_iterator wit = wedge_pairs.begin();
#if 0
        while (!wit->total_distance)
            ++wit;
#endif
        return *wit; */
    }

    const WedgePair & getMaxDist () const
    {
        /*WedgePairVec::const_iterator wit = wedge_pairs.end();
        return *--wit; */
    }

    float getMinDistance () const
    {
        return getMinDist().total_distance;
    }

    float getMaxDistance () const
    {
        return getMaxDist().total_distance;
    }

    float getMinDistAngle () const
    {
        return getMinDist().angle;
    }

    float getMaxDistAngle () const
    {
        return getMaxDist().angle;
    }

    friend class ImageTool;
};

#endif // ? OLD_PARTICLE_WEDGE_SCHEME

struct PixelOffset
{
    int8_t x;
    int8_t y;
};

class ImageTool
{
private:
    uint8_t * my_pixels;
    uint32_t my_w;
    uint32_t my_h;
    uint32_t my_left_margin;
    uint32_t my_right_margin;
    uint32_t my_top_margin;
    uint32_t my_bottom_margin;
    ADMImage * my_outImage;
    uint32_t my_minArea;
    uint32_t my_maxArea;
    uint32_t my_flags;
    uint32_t debug;
    Particle my_particle;

    enum TracingDirection
    {
        // The order of the members here is important - several things depend
        // on it!

        DIRECTION_EAST,   // x + 1, y
        DIRECTION_SOUTH,  // x    , y + 1
        DIRECTION_WEST,   // x - 1, y
        DIRECTION_NORTH,  // x    , y - 1
        DIRECTION_COUNT
    };

    enum TurnDirection
    {
        // These values are designed to be added to TracingDirection values.
        TURN_RIGHT = 1,
        TURN_AROUND = 2, // reverse
        TURN_LEFT = 3,
    };

    static char directionLetters [];
    static PixelOffset leftOffset [DIRECTION_COUNT];
    static PixelOffset aheadOffset [DIRECTION_COUNT];

public:
    enum Flags
    {
        FLAG_NONE = 0,
        FLAG_DO_SHAPE = 1,
    };

    ImageTool (uint8_t * pixels, uint32_t w, uint32_t h, ADMImage * outImage = 0,
               uint32_t flags = 0)
        : my_pixels (pixels),
          my_w (w),
          my_h (h),
          my_left_margin (0),
          my_right_margin (w),
          my_top_margin (0),
          my_bottom_margin (h),
          my_outImage (outImage),
          my_flags (flags),
          debug (0)
    {
    }

    void setCropping (uint32_t left_crop, uint32_t right_crop,
                      uint32_t top_crop, uint32_t bottom_crop)
    {
        my_left_margin = left_crop;
        my_right_margin = my_w - right_crop;
        my_top_margin = top_crop;
        my_bottom_margin = my_h - bottom_crop;
    }

    // Return true if the pixel is part of a particle.

    bool goodPixel (uint8_t pixel) const
    {
        // HERE: If we wanted to incorporate thresholding into this step, or look
        // for black particles on a white background, this function would be the
        // place to make the change.

        return (pixel != 0);
    }

    uint8_t & getPixel (uint32_t x, uint32_t y) const
    {
        return my_pixels [(y * my_w) + x];
    }

    uint8_t & getPixel (uint32_t index) const
    {
        return my_pixels [index];
    }

    // This one does bounds checking, and forces references "off the edge" to
    // the nearest valid pixel - it's useful for convolutions.  It does not
    // currently respect the crop settings, only because we know that this
    // function is used only when those aren't set.

    uint8_t & getPixelSafely (int32_t x, int32_t y) const
    {
        if (x < 0)
            x = 0;
        else if (x >= my_w)
            x = my_w - 1;

        if (y < 0)
            y = 0;
        else if (y >= my_h)
            y = my_h - 1;

        return my_pixels [(y * my_w) + x];
    }

    uint8_t & getPixelSafely (uint32_t x, uint32_t y) const
    {
        return getPixelSafely (static_cast <int32_t> (x),
                               static_cast <int32_t> (y));
    }

    uint8_t & outPixel (uint32_t x, uint32_t y) const
    {
        return YPLANE (my_outImage) [(y * my_w) + x];
    }

    uint8_t & outUPixel (uint32_t x, uint32_t y) const
    {
        return UPLANE (my_outImage) [((y >> 2) * my_w) + (x >> 1)];
    }

    uint8_t & outVPixel (uint32_t x, uint32_t y) const
    {
        return VPLANE (my_outImage) [((y >> 2) * my_w) + (x >> 1)];
    }

    bool validPixel (uint32_t x, uint32_t y) const
    {
        // return (x < my_w && y < my_h);
        return (x >= my_left_margin && x < my_right_margin
                && y >= my_top_margin && y < my_bottom_margin);
    }

    bool goodPixel (uint32_t x, uint32_t y) const
    {
        // HERE: this might be a good place to implement ROI (region of
        // interest) if we want that.

        return (validPixel (x, y) && goodPixel (getPixel (x, y)));
    }

    uint32_t width () const
    {
        return my_w;
    }

    uint32_t height () const
    {
        return my_h;
    }

#if 0
    uint32_t particleArea () const
    {
        return my_particle.area;
    }

    float particleCentroidX () const
    {
        return my_particle.centroidX;
    }

    float particleCentroidY () const
    {
        return my_particle.centroidY;
    }
#endif

    const Particle & getParticle () const
    {
        return my_particle;
    }

    uint8_t autoOutline (uint32_t x, uint32_t y);

    enum ShowStuff
    {
        SHOW_NOTHING = 0,
        SHOW_INPUT          = 0x0001,
        SHOW_OUTLINE        = 0x0002,
        SHOW_WEDGE_PIXELS   = 0x0004,
        SHIFT_PAST_SHOW_FLAGS = 4, // leave room for one more for now
    };

    void showStuff (ShowStuff what) const;

    // implementation is in ADM_vidSwissArmyKnife.cpp - nothing else uses it
    // (currently).
/*
    template <class Oper, class Histo>
    void convolve (const std::vector <float> & kernel,
                   uint32_t kw, uint32_t kh, int32_t bias, const Oper & op,
                   const Histo & histogram_in); */

    void setDebug (uint32_t newDebug)
    {
        debug = newDebug;
    }

    void setMinArea (uint32_t newMinArea)
    {
        my_minArea = newMinArea;
    }

    void setMaxArea (uint32_t newMaxArea)
    {
        my_maxArea = newMaxArea;
    }
};

struct MenuMapping;
uint8_t DIA_particle (AVDMGenericVideoStream *in,
                      ADMVideoParticle * particlep,
                      PARTICLE_PARAM * param,
                      const MenuMapping * menu_mapping,
                      uint32_t menu_mapping_count);

#endif
