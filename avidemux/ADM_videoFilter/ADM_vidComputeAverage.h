/***************************************************************************
                          ADM_vidComputeAverage.h  -  compute average of
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
 
#ifndef __COMPUTEAVERAGE__
#define __COMPUTEAVERAGE__   

#include <map>


typedef struct COMPUTEAVERAGE_PARAM
{
    int32_t start_frame;
    int32_t end_frame; // -1 means last frame, -2 = last - 1, ...
    const char * output_file;
    int32_t bias;
    uint32_t display_mode;

} COMPUTEAVERAGE_PARAM;

class  ADMVideoComputeAverage : public AVDMGenericVideoStream
{

    enum DisplayMode
    {
        DISPLAYMODE_INVALID = 0,
        DISPLAYMODE_FRAME_MINUS_AVERAGE,
        DISPLAYMODE_AVERAGE,
        DISPLAYMODE_BLANK,
        DISPLAYMODE_COUNT
    };

 public:

    struct FileHeader
    {
        char magic [8]; // "DGCMimg" + (is_float ? "F" : "8")
        uint32_t width;
        uint32_t height;
    };
        

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

         uint32_t *    sums;
         uint32_t      frame_count;
         uint32_t      width;
         uint32_t      height;
         uint32_t      start_frame;
         uint32_t      end_frame;
         uint32_t      most_recent_frame;

         PersistentInfo ()
             : conf (0),
               oldConf (0),
               refCount (0),

               sums (0),
               frame_count (0),
               width (0),
               height (0),
               start_frame (0),
               end_frame (0),
               most_recent_frame (0)
         {
         }

        ~PersistentInfo ()
        {
            delete [] sums;
        }
     };

     typedef std::map <CONFcouple *, PersistentInfo *> PImap;
     static PImap pimap;

     PersistentInfo * myInfo;

     COMPUTEAVERAGE_PARAM *  _param;

     void write_output_file () const;

 public:
 		
     ADMVideoComputeAverage (AVDMGenericVideoStream *in, CONFcouple *setup);

     ~ADMVideoComputeAverage();

     virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t *len,
                                            ADMImage *data, uint32_t *flags);

     virtual uint8_t configure (AVDMGenericVideoStream *instream);
     virtual char * printConf (void);
     virtual uint8_t getCoupledConf (CONFcouple **couples);
							
 };
#endif
