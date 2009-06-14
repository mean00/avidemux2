#ifndef ADM_VID_TDEINT_PARAM_H
#define ADM_VID_TDEINT_PARAM_H
typedef struct TDEINT_PARAM
{
    
/*  mode:
  
      Sets the mode of operation.  Modes -2 and -1 require progressive input.

         -2 - double height using modified ELA
         -1 - double height using modified ELA-2
          0 - same rate output
          1 - double rate output (bobbing)

      default -  0  (int)  >>>>>>>FOR AVIDEMUX ONLY 0 IS SUPPORTED <<<<
*/
	int32_t mode; //int
	
/*
      Sets the field order of the video.

        -1 - use parity from AviSynth
         0 - bottom field first (bff)
         1 - top field first (tff)
     
      default -  -1  (int)
      */
	int32_t order; // int


/*
      When in mode 0, this sets the field to be interpolated (i.e. the other field is kept as 
      is and this field will be constructed).  When in mode 1 this setting does nothing.

        -1 - will set field to 1 if hints = false or 0 if hints is set to true
         0 - interpolate top field (keep bottom field)
         1 - interpolate bottom field (keep top field)
  
      default -  -1  (int)

*/
	int32_t field; //int
/*
   mthreshL/mthreshC:

      The motion thresholds for luma and chroma (mthreshL for luma, mthreshC for chroma). If
      the difference between two pixels is less then this value they are declared static.
      Smaller values will reduce residual combing, larger values will decrease flicker and
      increase the accuracy of field construction in static areas.  The spatially corresponding
      parts of the luma and chroma planes are linked (if link != 0), so mthreshC and mthreshL 
      may be somewhat interconnected.  Setting both values to 0 or below will disable motion 
      adaptation (i.e. every pixel will be declared moving) allowing for a dumb bob.

      default - mthreshL - 6  (int)
                mthreshC - 6


		*/
	uint32_t mthreshL,mthreshC; //int
/*
	 map:

      Displays an output map instead of the deinterlaced frame.  There are three possible
      options.  **Note: the maps will not be displayed if the current frame is not being 
      deinterlaced due to overrides, hints, full=false, or tryWeave=true.

      **AP post-processing is currently not taken into account when using map = 1 or 2.

         0 - no map
         1 - value (binary) map.  This will output a frame in which all the pixels
             have one of the following values (indicating how the frame is to be
             constructed):
                 0   (use pixel from current frame)
                 51  (use pixel from previous frame)
                 102 (use pixel from next frame)
                 153 (use average of curr/next)
                 204 (use average of curr/prev)
                 255 (interpolate pixel)
         2 - merged map.  This will output a frame in which all the static parts of the 
             frame (values 0, 51, 102, 153, and 204 from map=1) have been constructed 
             as they would appear in the deinterlaced frame, and the pixels that are to be 
             interpolated are marked in white.

      default - 0  (int)
*/
	uint32_t map; //int
	/*
	   type:

      Sets the type of interpolation to use.  Cubic is the fastest, modified ELA and ELA2 will give
      smoother, less "jaggy", edges and are the slowest (ELA2 is faster), and kernel interpolation 
      will cause significantly less flickering then cubic or ela when interpolation gets used in 
      almost static areas.  Modified ELA/ELA2 works best with anime/cartoon type material... it is not
      that great with real life sources (sometimes it is, test for yourself).

         0 - cubic interpolation
         1 - modified ELA interpolation
         2 - kernel interpolation (can be normal or sharp, controlled by the sharp setting)
         3 - modified ELA-2 interpolation

      default - 2  (int)
      */
	uint32_t type; //
/*
	  debug:

      Will enable debug output, which for each frame will list the values of order, field, 
      mthreshL, mthreshC, and type if the frame is being deinterlaced.  If the frame is
      not being deinterlaced (due to user overrides, hints, or full=false) it will simply 
      say the frame is not being deinterlaced and list the specific reason. If the output
      frame is weaved, then debug output will report which field the current field was
      weaved with (PREV or NEXT).

      default - false  (bool)
      */
	uint32_t debug; // int
	/*
 mtnmode:

      Controls whether a 4 field motion check or a 5 field motion check is used.  5 field 
      will prevent more artifacts and can deal with duplicate interlaced frames, however
      it is quite a bit slower then the 4 field motion check.  Modes 2 and 3 are like 0 and
      1 except that in areas where an average of the prev and next field would have been
      used in mode 0 or 1, the pixel value from the most similar field (computed via field 
      differencing) is used instead (i.e. no averages are used).

         0 - 4 field check
         1 - 5 field check
         2 - 4 field check (no averages, replace with most similar field)
         3 - 5 field check (no averages, replace with most similar field)

      default - 1  (int)
*/
	uint32_t mtnmode;
/*
   sharp:

      Controls whether the sharp or normal kernel is used when using kernel interpolation (type
      = 2).  The sharp kernel includes more pixels and produces a sharper result, but is
      slightly slower.
 
         true - use sharp kernel
         false - use normal kernel

      default - true  (bool)
*/
	uint32_t sharp;
	/*
	  full:

      If full is set to true, then all frames are processed as usual.  If full=false, all frames
      are first checked to see if they are combed.  If a frame isn't combed then it is returned 
      as is.  If a frame is combed then it is processed as usual.  The parameters that effect 
      combed frame detection are cthresh, chroma, blockx, blocky, and MI.  Full=false allows 
      TDeint to be an ivtc post-processor without the need for hints.

         true - normal processing
         false - check all input frames for combing first
  
      default - true  (bool)
      */
	uint32_t full;
	/*
 cthresh:

      Area combing threshold used for combed frame detection.  It is like dthresh or dthreshold
      in telecide() and fielddeinterlace().  This essentially controls how "strong" or "visible"
      combing must be to be detected.  Good values are from 6 to 12, if you know your source has 
      a lot of combed frames set this towards the low end (6-7), if you know your source has
      very few combed frames set this higher (10-12).  Going much lower then 5 to 6 or much 
      higher then 12 is not recommended.

      default - 6  (int)
*/
	uint32_t cthresh;
/*
   blockx -

      Sets the x-axis size of the window used during combed frame detection.  This has to do with 
      the size of the area in which MI number of pixels are required to be detected as combed for 
      a frame to be declared combed.  See the MI parameter description for more info.  Possible 
      values are any number that is a power of 2 starting at 4 and going to 2048 (i.e. 4, 8, 16, 
      32, ... 2048).

      Default:  16  (int)
*/
	uint32_t blockx;
/*
   blocky -

      Sets the y-axis size of the window used during combed frame detection.  This has to do with 
      the size of the area in which MI number of pixels are required to be detected as combed for 
      a frame to be declared combed.  See the MI parameter description for more info.  Possible 
      values are any number that is a power of 2 starting at 4 and going to 2048 (i.e. 4, 8, 16, 
      32, ... 2048).

      Default:  16  (int)
*/
	uint32_t blocky;
 /* 
   chroma:

      Includes chroma combing in the decision about whether a frame is combed.  Only use this if
      you have one of those weird sources where the chroma can be temporally separated from the luma
      (i.e. the chroma moves but the luma doesn't in a field).  Otherwise it will just end up 
      screwing up the decision most of the time.

         true - include chroma combing
         false - don't
 
      default - false  (bool)
      */
	uint32_t chroma;
/*
	 MI:

      # of combed pixels inside any of the blockx by blocky sized blocks on the frame for the 
      frame to be considered combed.  While cthresh controls how "visible" or "strong" the combing 
      must be, this setting controls how much combing there must be in any localized area (a 
      blockx by blocky sized window) on the frame.  Min setting = 0, max setting = blockx x blocky
      (at which point no frames will ever be detected as combed).

      default - 64  (int)
*/
	uint32_t MI;
/*
   tryWeave:

      If set to true, when TDeint deinterlaces a frame it will first calculate which field
      (PREV or NEXT) is most similar to the current field.  It will then weave this field
      to create a new frame and check this new frame for combing.  If the new frame is not
      combed, then it returns it. If it is, then it deinterlaces using the usual per-pixel
      motion adaptation.  Basically, this setting allows TDeint to try to use per-field
      motion adaptation instead of per-pixel motion adaptation where possible.

      default - false  (bool)
*/
	uint32_t tryWeave;
  /* 
   link:

      Controls how the three planes (YUV) are linked during comb map creation. Possible
      settings:

        0 - no linking
        1 - Full linking (each plane to every other)
        2 - Y to UV (luma to chroma)
        3 - UV to Y (chroma to luma)

      default - 2  (int)
*/
	uint32_t link;
/*
   denoise:

      Controls whether the comb map is denoised or not.  True enables denoising, false disables.

      default - true  (bool)
*/
	uint32_t denoise;
/*
   AP:

      Artifact protection threshold.  If AP is set to a value greater than or equal to 0, then
      before outputting a deinterlaced frame TDeint will scan all weaved pixels to see if any
      create a value greater then AP.  Any pixels that do will be interpolated.  Use this to
      help prevent very obvious motion adaptive related artifacts, a large value is recommended
      (25+, or as large as removes the artifacts that can be seen during full-speed playback) as 
      smaller values will destroy the benefits of motion adaptivity in static, detailed areas.
      The AP metric is the same as the cthresh metric so the scale is 0-255... at zero everything
      but completely flat areas will be detected as combing, at 255 nothing will be detected.
      Using AP will slow down processing. Set AP to a value less then 0 or greater than
      254 to disable.

      default - -1 (disabled)  (int)

*/
	int32_t AP;
	/*
   APType

      When AP post-processing is being used (AP is set >= 0 and < 255), APType controls whether
      the motion of surrounding pixels should be taken into account.  There are 3 possible 
      settings:

        0 = Don't take surrounding motion into account.  If a weaved pixel creates a value that
            exceeds the AP threshold then it will be interpolated.

        1 = If a weaved pixel creates a value that exceeds the AP threshold and at least half of
            pixels in a 5x5 window centered on that pixel were detected as moving, then that
            pixel will be interpolated.

        2 = Exactly like 1, except instead of 1/2 only 1/3 of the pixels in the surrounding 5x5 
            window must have been detected as moving.

      Modes 1 and 2 provide a way to catch more artifacts (low AP values) without completely 
      sacrificing static areas.


      default -  1  (int)
         */
	uint32_t APType;

}TDEINT_PARAM;
#endif
//EOF








