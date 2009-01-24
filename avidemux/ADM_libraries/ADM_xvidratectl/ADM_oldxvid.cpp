//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "avi_vars.h"
#include "ADM_assert.h"
#include "ADM_ratecontrol.h"

ADM_ratecontrol::ADM_ratecontrol(uint32_t fps1000, char *logname)
{
	_fps1000=fps1000;
	_logname=ADM_strdup(logname);
	_state=RS_IDLE;
	_nbFrames++;
}

ADM_ratecontrol::~ADM_ratecontrol()
{
	delete _logname;
}
#if 0 // RMED
extern "C" {
#include "ADM_encoder/xvid_vbr.h"
};


ADM_oldXvidRc::~ADM_oldXvidRc()
{
	if(_state!=RS_IDLE)
	{
		vbrFinish(&mpegvbr);
	}
	_state=RS_IDLE;
}
//
ADM_oldXvidRc::ADM_oldXvidRc(uint32_t fps1000, char *logname) : ADM_ratecontrol(fps1000,logname)
{
	memset(&mpegvbr,0,sizeof(mpegvbr));
	memset(&mpegvbr,0,sizeof(mpegvbr));
        ADM_assert(! vbrSetDefaults(&mpegvbr));
	mpegvbr.fps=_fps1000/1000.;

 	mpegvbr.mode=VBR_MODE_2PASS_1;

	mpegvbr.debug=0;
	mpegvbr.filename=_logname; //XvidInternal2pass_statfile;

	ADM_assert(! vbrInit(&mpegvbr));	
}
//
uint8_t ADM_oldXvidRc::setVBVInfo(uint32_t maxbr,uint32_t minbr, uint32_t vbvsize)
{

	return 1;
}
uint8_t ADM_oldXvidRc::startPass1( void )
{

	ADM_assert(_state==RS_IDLE);
	_state=RS_PASS1;
	return 1;
}
//
uint8_t ADM_oldXvidRc::logPass1(uint32_t qz, ADM_rframe ftype,uint32_t size)
{
int intra=0;
	if(ftype==RF_I) intra=1;
	_nbFrames++;
	if(vbrUpdate(&mpegvbr,
				qz,
				intra,
				0,
				size,
				0,
				0,
				(int)ftype)) return 0;
		
	return 1;
}
//
uint8_t ADM_oldXvidRc::startPass2( uint32_t final_size ,uint32_t nbFrame)
{
uint64_t total_size;

	ADM_assert((_state==RS_IDLE)||(_state==RS_PASS1));
	if(_state==RS_PASS1)
	{
		vbrFinish(&mpegvbr);
	}
	_state=RS_PASS2;
	
	
	total_size=0;
	 memset(&mpegvbr,0,sizeof(mpegvbr));
	ADM_assert(! vbrSetDefaults(&mpegvbr));
	mpegvbr.fps=_fps1000/1000.;

	mpegvbr.mode=VBR_MODE_2PASS_2;
	mpegvbr.desired_size=(uint64_t)final_size*(uint64_t)1024*(uint64_t)1024;
	mpegvbr.debug=0;
	mpegvbr.filename=_logname; //XvidInternal2pass_statfile;

	float br;
	uint32_t avg;
	_nbFrames=nbFrame;
	ADM_assert(_nbFrames);
	br=mpegvbr.desired_size*8;
	br=br/_nbFrames;				// bit / frame
	br=br*mpegvbr.fps;

	mpegvbr.desired_bitrate= (int)floor(br);
	avg=(uint32_t)floor(br/1000.);
	printf("XVIDRC:(old2)average bitrate : %lu\n",avg);
	//mpegvbr.maxAllowedBitrate=(2500*1000)>>3; 
	// enable stuff in xvid
	//mpegvbr.twopass_max_bitrate=2500*1000;
	//mpegvbr.alt_curve_high_dist=10;
	//mpegvbr.alt_curve_low_dist=30;
	//mpegvbr.alt_curve_type=VBR_ALT_CURVE_AGGRESIVE;

	if(0>vbrInit(&mpegvbr))
	{
		return 0;
	}
	
	return 1;

}
uint8_t ADM_oldXvidRc::getQz( uint32_t *qz, ADM_rframe *type )
{
	*qz=vbrGetQuant(&mpegvbr);
	if( vbrGetIntra(&mpegvbr)) 	*type=RF_I;
		else			*type=RF_P;
	return 1;
}
uint8_t ADM_oldXvidRc::logPass2( uint32_t qz, ADM_rframe ftype,uint32_t size)
{
int intra;
	if(ftype==1) intra=1;
		else	intra=0;
	vbrUpdate(&mpegvbr,
				qz,
				//q,
				intra,
				0,
				size,
				0,
				0,
				ftype);
	return 1;



}
#endif

//EOF

