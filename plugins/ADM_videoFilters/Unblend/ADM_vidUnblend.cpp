// Port of Bach unblend avisynth filter
//
//      Copyright by Bach, D Graft, Ernst Pech�
//      Probably GPL
//
//

#include <math.h>

#include "ADM_default.h"
#include "ADM_videoFilterDynamic.h"

#include "ADM_vidUnblend_param.h"

class vidUnblend:public AVDMGenericVideoStream
{

protected:
  virtual char *printConf (void);
  VideoCache *vidCache;
  PARAM_UNBLEND *_param;

  void DrawShow(ADMImage *src, int useframe, int blend, int inframe, int threshold, float dthresh);
  void Findblend(int frame, int *bblend);
  int  firsttime;
  int  reference;
  unsigned int *sum;

  
public:

                        vidUnblend (AVDMGenericVideoStream * in, CONFcouple * setup);
        virtual         ~vidUnblend ();
  virtual uint8_t getFrameNumberNoAlloc (uint32_t frame, uint32_t * len,
                                         ADMImage * data, uint32_t * flags);
  uint8_t configure (AVDMGenericVideoStream * instream);
  virtual uint8_t getCoupledConf (CONFcouple ** couples);

};

static FILTER_PARAM unblend_template =
  { 3,"show","threshold","dthresh" };

//********** Register chunk ************
VF_DEFINE_FILTER(vidUnblend,unblend_template,
                unblend,
                QT_TR_NOOP("Median (5x5)"),
                1,
                VF_NOISE,
                QT_TR_NOOP("Unblend by Bach"));
//****************************************
//*************************************
uint8_t vidUnblend::configure (AVDMGenericVideoStream * in)
{
       
        return 0;
}
/*************************************/
char *vidUnblend::printConf (void)
{
	ADM_FILTER_DECLARE_CONF(" Unblend by Bach");
}
uint8_t vidUnblend::getCoupledConf (CONFcouple ** couples)
{

  ADM_assert (_param);
  *couples = new CONFcouple (3);
#undef CSET
#define CSET(x)  (*couples)->setCouple(#x,(_param->x))

  CSET (show);
  CSET (threshold);
  CSET (dthresh);
  return 1;
}

#define MAX_BLOCKS 50
/*************************************/
vidUnblend::vidUnblend (AVDMGenericVideoStream * in, CONFcouple * couples)
{

  _in = in;
  memcpy (&_info, _in->getInfo (), sizeof (_info));
  _info.encoding = 1;
  vidCache = new VideoCache (10, in);
  _param=new PARAM_UNBLEND;
  if(couples)
  {
#undef GET
#define GET(x) couples->getCouple(#x,&(_param->x))
      GET (show);
      GET (threshold);
      GET (dthresh);

  }
  else
  {
                _param->show=1;
                _param->threshold=5;
                _param->dthresh=3./100.;

  }
  firsttime=true;
  sum=new unsigned int[MAX_BLOCKS * MAX_BLOCKS];
  reference=0;
  DrawShow(NULL,0,0,0,_param->threshold,_param->dthresh);
  
}
//____________________________________________________________________
vidUnblend::~vidUnblend ()
{

  delete vidCache;
  vidCache = NULL;
  delete _param;
  _param=NULL;
  delete [] sum;
  sum=NULL;

}


#define DrawString(x,y,z,t) printf("Unblend :%s\n",t);
#define GETFRAME(useframe, src) src=vidCache->getImage(useframe)
void vidUnblend::DrawShow(ADMImage *src, int useframe, int blend, int inframe, int threshold, float dthresh)
{
	char buf[80];

	if (_param->show == true)
	{
		sprintf(buf, "unblend %s", 0);
		DrawString(src, 0, 0, buf);
		sprintf(buf, "Copyright 2003 Bach, based on source by D.Graft and Ernst Pech");
		DrawString(src, 0, 1, buf);
		sprintf(buf,"Using frm %d, instead of frm %d", useframe, useframe-blend);
		DrawString(src, 0, 2, buf);
		sprintf(buf,"the diff threshold is %d", threshold);
		DrawString(src, 0, 4, buf);	
		sprintf(buf,"the noise threshold is  %f", dthresh);
		DrawString(src, 0, 5, buf);	
	}
}

uint8_t vidUnblend::getFrameNumberNoAlloc (uint32_t inframe,
                                uint32_t * len,
                                ADMImage * data, uint32_t * flags)
{

	int blend=0, useframe;
	ADMImage *src;
	char buf[255];
        if(inframe>= _info.nb_frames) return 0;
        if(!inframe || inframe>_info.nb_frames-5)
        {
                data->duplicate(vidCache->getImage(inframe));
                vidCache->unlockAll();
                return 1;
        }

	useframe = inframe;
	Findblend(useframe, &blend);
	//useframe++; ?????
	switch(blend)
		{
		case (-1):
			useframe--;
			break;
		case 1:
			useframe++;
			break;
		default:
			break;
		}
	        
	   	

                if (_param->show == true)
		{
/*
                sprintf(buf, "unblend %s", VERSION);
		DrawString(src, 0, 0, buf);
		sprintf(buf, "Copyright 2003 Bach, based on source by D.Graft and Ernst Pech�);
		DrawString(src, 0, 1, buf);
*/
		sprintf(buf,"Using frm %d, instead of frm %d\n*****************", useframe, useframe-blend);
		DrawString(src, 0, 2, buf);
/*
		sprintf(buf,"the diff threshold is %d", _param->threshold);
		DrawString(src, 0, 4, buf);	
		sprintf(buf,"the noise threshold is  %f", _param->dthresh);
		DrawString(src, 0, 5, buf);		
*/
		}
        src=vidCache->getImage(useframe);
        data->duplicate(src);
        vidCache->unlockAll();
	return 1;
}

void  vidUnblend::Findblend(int frame, int *bblend)

{
	int f, x, y,pitchY,row_sizeY,heightY;
	ADMImage *store[5];
	const unsigned char *storepY[5];
	float t;
	float metric02, metric13, metric24;
	int metric_count02, metric_count13, metric_count24;
	int count02, count13, count24;
	float p0, p1, p2, p3, p4;
	float percent02, percent13, percent24;
        int vi_height=_info.height;
        int vi_width=_info.width;
		
	if (firsttime == true || frame == 0)
	{
		firsttime=false;
		reference=0;
	}

	for (f = 0; f <= 4; f++)
	{
		GETFRAME(frame + f - 1, store[f]);
		storepY[f] = store[f]->data; //store[f]->GetReadPtr(PLANAR_Y);
	}

    pitchY = _info.width;    //store[0]->GetPitch(PLANAR_Y);
    row_sizeY = _info.width; //store[0]->GetRowSize(PLANAR_Y);
    heightY = _info.height;  //store[0]->GetHeight(PLANAR_Y);
		
	metric_count02=1;
	metric02=0;
	metric_count13=1;
	metric13=0;
	metric_count24=1;
	metric24=0;
	count02=0;
	count13=0;
	count24=0;

	for ( y=0; y<vi_height-1; y=y+2) 
	{
	    for ( x=0; x<row_sizeY-1; x=x+2) {
			
			p0 = (float)(storepY[0][x] + storepY[0][x+pitchY])/2;
			p1 = (float)(storepY[1][x] + storepY[1][x+pitchY])/2;
			p2 = (float)(storepY[2][x] + storepY[2][x+pitchY])/2;
			p3 = (float)(storepY[3][x] + storepY[3][x+pitchY])/2;
			p4 = (float)(storepY[4][x] + storepY[4][x+pitchY])/2;
			
			t=1000;
			if (fabs(p1- p3)>_param->threshold ) {
				t = 255 * (p2 - p1)/(p3 - p1);	// tmax = +-255/1   tmin = +-1/255
				if (t>32 && t<224) {
					count13++;
					metric_count13++;
					metric13 = metric13 + t;
				} else {
					count13--;
				}
			}

			// prev frame
			if (fabs(p0 - p2)>_param->threshold ) {
				t = 255 * (p1 - p0)/(p2 - p0);
				if (t>32 && t<224) {
					count02++;
					metric02 = metric02 + t;
					metric_count02++;
				} else {
					count02--;
				}
			}
			// next frame
			if (fabs(p2 - p4)>_param->threshold ) {
				t = 255 * (p3 - p2)/(p4 - p2);
				if (t>32 && t<224) {
					count24++;
					metric24 = metric24 + t;
					metric_count24++;
				} else {
					count24--;
				}
			}
		}

		storepY[0] += 2 * pitchY;
		storepY[1] += 2 * pitchY;
		storepY[2] += 2 * pitchY;
		storepY[3] += 2 * pitchY;
		storepY[4] += 2 * pitchY;
	}

	metric02 = metric02 / metric_count02;
	metric13 = metric13 / metric_count13;
	metric24 = metric24 / metric_count24;

	percent02 = (float)200 * count02 / (vi_height * vi_width);
	percent13 = (float)200 * count13 / (vi_height * vi_width);
	percent24 = (float)200 * count24 / (vi_height * vi_width);

        if(_param->show)
        {
                printf("PC02:%f PC13:%f PC24:%f\n",percent02,percent13,percent24);
                printf("Metric13:%f \n",metric13);
        }

	if (percent13>_param->dthresh && (percent02<_param->dthresh ) && (percent24<_param->dthresh )) 
	{
                if(_param->show) printf("BLENDED\n");
		if (metric13<128 && metric13>32) reference=(-1);
		else if(metric13>128 && metric13<224) reference=1;
	}
	else reference=0;
	
	*bblend = reference;
	
}
//EOF
