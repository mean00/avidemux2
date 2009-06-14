/***************************************************************************
                          DIA_flyThreshold.cpp  -  configuration dialog for
						   threshold filter
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

#ifndef FLY_THRESHOLD_H
#define FLY_THRESHOLD_H

class flyThreshold : public ADM_flyDialog
{
  
  private:
    ADMVideoThreshold * thresholdp;
    THRESHOLD_PARAM saved_param;
    THRESHOLD_PARAM * live_param;
    AVDMGenericVideoStream * source;
    THRESHOLD_PARAM param;

  public:

    // HERE: A lot of this should probably be moved up into ADM_flyDialog

    uint16_t this_filter_index;

    enum PreviewMode
    {
        PREVIEWMODE_INVALID = 0, // never use
        PREVIEWMODE_EARLIER_FILTER,
        PREVIEWMODE_THIS_FILTER,
        PREVIEWMODE_LATER_FILTER
    };

  private:
    PreviewMode preview_mode;

  public:
    uint8_t    process(void);
    uint8_t    download(void);
    uint8_t    upload(void);
    uint8_t    update(void);

    flyThreshold (uint32_t width, uint32_t height,
                  AVDMGenericVideoStream * in,
                  void * canvas, void * slider,
                  ADMVideoThreshold * thresholdp,
                  THRESHOLD_PARAM * in_param)
        : ADM_flyDialog (width, height, in, canvas, slider, 1, RESIZE_AUTO),
          thresholdp (thresholdp),
          saved_param (*in_param),
          live_param (in_param),
          source (thresholdp),
          param (*in_param),
          this_filter_index (0),
          preview_mode (PREVIEWMODE_THIS_FILTER)
    {
    };

    uint8_t sliderChanged (void);

    void pushParam ()
    {
        *live_param = param;
    }

    // no longer used
    void getParam (THRESHOLD_PARAM * outputParam)
    {
        *outputParam = param;
    }

    void restoreParam ()
    {
        *live_param = saved_param;
    }

    AVDMGenericVideoStream * getSource () const
    {
        return source;
    }

    void changeSource (AVDMGenericVideoStream * in, PreviewMode mode)
    {
        source = in;
        preview_mode = mode;
        sliderChanged();
    }

    ADMImage * getInputImage ()
    {
        return _yuvBuffer;
    }

    ADMImage * getOutputImage ()
    {
        return _yuvBufferOut;
    }

    float getZoom () const
    {
        return _zoom;
    }
};

#endif
