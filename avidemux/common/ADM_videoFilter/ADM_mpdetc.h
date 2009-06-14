//
//
// C++ Interface: ADM_mpdetc
//
// Description: 
//		Port of Mplayer Detc filter (inverse telecine)
//		Original author & copyright : Richard Felker
//
// Author: mean <fixounet@free.fr>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
typedef struct metrics {
	/* difference: total, even lines, odd lines */
	int d, e, o;
	/* noise: temporal, spacial (current), spacial (past) */
	int t, s, p;
};

typedef struct frameinfo {
	/* peak, relative, mean */
	 metrics p, r, m;
};

struct vf_priv_s {
	struct frameinfo fi[2];
	uint8_t  *dmpi;
	int first;
	int drop, lastdrop, dropnext;
	int inframes, outframes;
};

enum {
	F_DROP,
	F_MERGE,
	F_NEXT,
	F_SHOW
};



class  AVDMVideoMPDetc:public AVDMGenericVideoStream
 {

 protected:

	struct vf_priv_s	*_param;
	ADMImage		*_lastFrame;
        virtual char 		*printConf(void) ;
	
	
		uint8_t 	do_put_image(ADMImage *data);
		uint32_t	_inFrame;
		uint32_t	_lastAsked;
		uint32_t 	_copy;
	

 public:
  				AVDMVideoMPDetc(  AVDMGenericVideoStream *in,CONFcouple *setup);
  				~AVDMVideoMPDetc();
	virtual uint8_t 	getFrameNumberNoAlloc(uint32_t frame, uint32_t *len,
          									ADMImage *data,uint32_t *flags);

			virtual uint8_t 	configure( AVDMGenericVideoStream *instream) 
								{return 0;};


 }     ;

 
