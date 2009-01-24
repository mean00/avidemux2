/***************************************************************************
                          ADM_vidSRT.h  -  description
                             -------------------
    begin                : Thu Dec 12 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
    
    Lots of bugfixes / enhancement by Daniel Lima
    
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SRT_MAX_LINE 3
#define SRT_MAX_LINE_LENGTH 512

#define ADM_GLYPH_T uint16_t 
// UTF16 -> Ascii

#define ADM_ASC(x) x

typedef struct subLine
{
  
	uint32_t 		startTime;
	uint32_t 		endTime;
	uint32_t		nbLine;
	uint32_t		*lineSize;
	ADM_GLYPH_T		**string;
	
}subLine;

typedef enum 
{
        BLEND_SOLID=1,
        BLEND_DOTTED,
        BLEND_DIMMER
}BlendMode;

typedef struct SUBCONF
{
		uint32_t _fontsize;
		uint32_t _baseLine;

  /** YUV font color components */
		int32_t    _Y_percent;
		int32_t    _U_percent;
		int32_t    _V_percent;
		ADM_filename	*_fontname;
		ADM_filename	*_subname;
		char 		*_charset;
		uint32_t	_selfAdjustable;   /** Automatic line breaks */
		int32_t		_delay;            
		/** Wait x miliseconds before show subtitles. If negative, it will to show subtitles x */
		/* miliseconds before. */


		uint32_t	_useBackgroundColor;

  /** YUV background color components */
		int32_t   _bg_Y_percent;
		int32_t   _bg_U_percent;
		int32_t   _bg_V_percent;
                BlendMode _blend;


	}SUBCONF;

  class  ADMVideoSubtitle:public AVDMGenericVideoStream
 {

 protected:

	    	SUBCONF					*_conf;		
       virtual char 					*printConf(void) ;
        FILE						*_fd;
        uint8_t						loadSubTitle( void );
	uint8_t  					loadSRT( void )        ;
        uint32_t					_line;
        subLine						*_subs;
        uint32_t					_oldframe;
        uint32_t					_oldline;
        uint32_t					_bitmap;
        uint32_t					search(uint32_t time);
        void 						displayString(subLine *string);
	void 						displayString_autoadj(char *string);
        void 						displayChar(uint32_t w,uint32_t h,char c);
  	uint32_t					displayLine(ADM_GLYPH_T *string,uint32_t line, uint32_t len);
  
	uint8_t 					lowPass(uint8_t *src, uint8_t *dst, uint32_t w, uint32_t h);
	uint8_t 					decimate(uint8_t *src, uint8_t *dst, uint32_t w, uint32_t h);


        uint8_t						*_bitmapBuffer;
	uint8_t						*_maskBuffer;
	uint8_t						*_bgBitmapBuffer;
	uint8_t						*_bgMaskBuffer;
        uint8_t                                         *_dirty;
	uint8_t						blend(uint8_t *target,uint32_t baseLine);
	ADMfont						*_font;
	uint8_t						loadSubtitle(void);
	uint8_t						loadFont(void);
	uint8_t 					subParse( subLine *in, char *string );
        uint8_t                                         doChroma(void);
        uint8_t                                         clearBuffers(void);
        void                                            doAutoSplit(subLine *string);
        uint8_t                                         isDirty(int line);

 public:
 		
  						ADMVideoSubtitle(  AVDMGenericVideoStream *in,CONFcouple *setup);
  						~ADMVideoSubtitle();
		      virtual uint8_t 		getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
       								ADMImage *data,uint32_t *flags);

			virtual uint8_t	getCoupledConf( CONFcouple **couples)		;
			virtual uint8_t 	configure( AVDMGenericVideoStream *instream);
							
 }     ;
