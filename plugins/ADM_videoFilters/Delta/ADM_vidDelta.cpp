//
//      Try to detect blended image
//      Mean (C) 2005
//      GPL v2.0
//


#include "ADM_default.h"

#include "ADM_videoFilterDynamic.h"
#define aprintf(...) {}


class vidDelta:public AVDMGenericVideoStream
{

protected:
  virtual char *printConf (void);
  VideoCache *vidCache;


public:

                        vidDelta (AVDMGenericVideoStream * in, CONFcouple * setup);
        virtual         ~vidDelta ();
  virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t * len,
					 ADMImage * data, uint32_t * flags);
  uint8_t configure (AVDMGenericVideoStream * instream);
  virtual uint8_t getCoupledConf (CONFcouple ** couples);

};

static FILTER_PARAM delta_template =
  { 0 };

//********** Register chunk ************

VF_DEFINE_FILTER(vidDelta,delta_template,
                delta,
                QT_TR_NOOP("Luma delta"),
                1,
                VF_COLORS,
                QT_TR_NOOP("Difference between current and previous picture."));
//********** Register chunk ************

/*************************************/
uint8_t vidDelta::configure (AVDMGenericVideoStream * in)
{
        return 1;
}
/*************************************/
char *vidDelta::printConf (void)
{
 ADM_FILTER_DECLARE_CONF( " Delta");
  
}
/*************************************/
vidDelta::vidDelta (AVDMGenericVideoStream * in, CONFcouple * couples)
{

  _in = in;
  memcpy (&_info, _in->getInfo (), sizeof (_info));
  _info.encoding = 1;
  vidCache = new VideoCache (4, in);
  
  
}
//____________________________________________________________________
vidDelta::~vidDelta ()
{

  delete vidCache;
  vidCache = NULL;

}

//______________________________________________________________
uint8_t vidDelta::getFrameNumberNoAlloc (uint32_t frame,
				uint32_t * len,
				ADMImage * data, uint32_t * flags)
{
int w,h,page;
ADMImage *cur,*prev,*next;
        w=_info.width;
        h=_info.height;
        page=w*h;
  if(frame>= _info.nb_frames) return 0;
  if(!frame || frame>=_info.nb_frames-1)
  {
        memset(YPLANE(data),0,page);
        memset(UPLANE(data),128,page>>2);
        memset(VPLANE(data),128,page>>2);
        return 1;
  }
  cur=vidCache->getImage (frame);
  prev=vidCache->getImage (frame-1);
  next=vidCache->getImage (frame+1);
  // No chroma
        memset(UPLANE(data),128,page>>2);
        memset(VPLANE(data),128,page>>2);

   uint8_t *c=YPLANE(cur);
   uint8_t *p=YPLANE(prev);
   uint8_t *n=YPLANE(next);
   uint8_t *out=YPLANE(data);
#define SAME_THRESH 32
   int a,b,d,cp,cn,e;
   for(int y=0;y<h;y++)
        for(int x=0;x<w;x++)
        {
                a=*c++;
                b=*p++;
                e=*n++;
                cp=abs(a-b);
                cn=abs(a-e);
                // Cp is the != between current and previous
                // cn is the != between current and next
                // We try to spo frames that are blended betwwen next & previous
                
                if(cp>SAME_THRESH && cn > SAME_THRESH && abs(cp-cn) < 128) *out++=255;//(cp+cn)/2;
                        else *out++=0;
        }


  vidCache->unlockAll ();
  return 1;
}

/*************************************/
uint8_t vidDelta::getCoupledConf (CONFcouple ** couples)
{

  *couples=NULL;
  return 1;
}

// EOF
