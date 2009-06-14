/***************************************************************************
                          ADM_vidEraser.h  -  "Erase" arbitrary areas of
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
 
#ifndef __ERASER__
#define __ERASER__   

#include <map>


#include <vector>
#include <string>

#include <sys/time.h>

struct ERASER_PARAM
{
    uint32_t brush_mode; // 0 = un-erase, 1 = erase
    uint32_t brush_size; // NxN where N = brush_size * 2 + 1
    uint32_t output_color; // 0 - 255
    std::string data_file;
    uint32_t debug;
};

// Alas, because offsetof() is only supposed to work on POD (plain old data)
// structs, and our ERASER_PARAM includes a std::string (which has a
// constructor, and which causes ERASER_PARAM to therefore have an implicit
// constructor), we need to define our own offsetof() to use for the dialog
// menus.  See
// http://www.cplusplus.com/reference/clibrary/cstddef/offsetof.html for more
// on offsetof().

#define my_offsetof(_type, _memb) (size_t (&(((_type *)1)->_memb)) - 1)

namespace Eraser
{
    // The mask for a range of frames is represented as a list (vector) of
    // lines, where each line is a horizontal line of pixels.  Thus, each Line
    // has a y coordinate, and a range of x coordinates (represented as the
    // leftmost x coordinate and a count of pixels).  The list of lines is
    // kept sorted by y and then by x, to make it more efficient to merge in
    // new pixels or find ones to remove while maintaining the most compact
    // (and thus most efficient to apply to an image) representation.

    struct Line
    {
        uint16_t x;
        uint16_t y;
        uint16_t count;

        Line ()
            : x(), y(), count()
        {
        }

        Line (uint16_t x, uint16_t y, uint16_t count)
            : x (x), y (y), count (count)
        {
        }
    };

    typedef std::vector <Line> LineVec;

    struct Mask
    {
        uint32_t first_frame;
        uint32_t last_frame;
        LineVec lines;

        Mask (uint32_t first_frame, uint32_t last_frame)
            : first_frame (first_frame),
              last_frame (last_frame)
        {
        }

        Mask (uint32_t first_frame, uint32_t last_frame,
              const LineVec & lines)
            : first_frame (first_frame),
              last_frame (last_frame),
              lines (lines)
        {
        }
    };

    typedef std::vector <Mask> MaskVec;
}

class ADMVideoEraser : public AVDMGenericVideoStream
{
protected:
    	
    // This is a hack to work around the fact that the ctor & dtor get called
    // too often.  The right solution would be to arrange for the filter
    // objects to be constructed and destructed only when really necessary:
    // when a new instance of a filter is added to the list (by the user), it
    // is constructed, and when it is removed from the list (by the user), it
    // is destructed, and anything else is handled by a separate init() or
    // configure() method.  This would allow the objects to maintain a
    // persistent state in a more straightforward way.

    class PersistentInfo
    {
    public:
        CONFcouple *  conf;
        CONFcouple *  oldConf;
        uint32_t      refCount;

//        uint32_t      mask_w;
//        uint32_t      mask_h;

//        std::string   data_file_name;
//        time_t        data_file_mtime;

        Eraser::MaskVec masks;

        bool          mask_data_invalid;

        PersistentInfo ()
            : conf (0),
              oldConf (0),
              refCount (0),

//              mask_w (0),
//              mask_h (0),

//              data_file_mtime (0),
              mask_data_invalid (true)
        {
        }
    };

    typedef std::map <CONFcouple *, PersistentInfo *> PImap;
    static PImap pimap;

    PersistentInfo * myInfo;

    ERASER_PARAM * _param;

public:

    ADMVideoEraser (AVDMGenericVideoStream * in, CONFcouple * setup);
    ~ADMVideoEraser();

    virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t * len,
                                           ADMImage * data, uint32_t * flags);

    virtual uint8_t configure (AVDMGenericVideoStream * instream);
    virtual char * printConf (void);
    virtual uint8_t getCoupledConf (CONFcouple ** couples);

    static uint8_t doEraser (ADMImage * from_image,
                             ADMImage * to_image,
                             AVDMGenericVideoStream * in,
                             uint32_t real_frame,
                             ADMVideoEraser * eraserp,
                             ERASER_PARAM * param,
                             uint32_t width, uint32_t height);

    Eraser::MaskVec & getMasks ()
    {
        return myInfo->masks;
    }

    void masksIsValid (bool valid)
    {
        myInfo->mask_data_invalid = !valid;
    }

protected:

    uint8_t readDataFile (uint32_t width);
    void writeDataFile () const;

private:

    const ADV_Info & getInfo () const
    {
        return _info;
    }
};

struct MenuMapping;
uint8_t DIA_eraser (AVDMGenericVideoStream * in,
                    ADMVideoEraser * eraserp,
                    ERASER_PARAM * param,
                    const MenuMapping * menu_mapping,
                    uint32_t menu_mapping_count);

#endif
