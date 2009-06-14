/***************************************************************************
                          ADM_vidPartial.cpp  -  description
                             -------------------
    begin                : Mon Dec 30 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr

    This filter is special
    It uses internally another filter from _start to _end
    and output copy of input else

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"

#include "ADM_editor/ADM_edit.hxx"
#include "ADM_videoFilter.h"
#include "ADM_videoFilter_internal.h"
#include "ADM_video/ADM_vidPartial.h"

extern AVDMGenericVideoStream *filterCreateFromTag(VF_FILTERS tag,CONFcouple *couple, AVDMGenericVideoStream *in);


static FILTER_PARAM partialParam={VARIABLE_PARAMS+3,{"_start","_end","_tag"}};


SCRIPT_CREATE(partial_script,ADMVideoPartial,partialParam);
BUILD_CREATE(partial_create,ADMVideoPartial);

//___________________________________________________
char 						*ADMVideoPartial::printConf(void)
{
	static char string[500];
		 			sprintf(string,"Partial from %"LD" to %"LD" : %s",
		    						_param->_start,
		          			_param->_end,
		             		_son->printConf());
		    return string;
}

ADMVideoPartial::ADMVideoPartial(   AVDMGenericVideoStream *in,
							CONFcouple		*couples)
{
		_param=NEW( PARTIAL_CONFIG);
		_param->_start=0;
		_param->_end=0;
		_in=in;

			GET(_start);
			GET(_end);
			GET(_tag);
			// we share the same parameters
			_son= filterCreateFromTag((VF_FILTERS)_param->_tag,couples,_in);
		 	memcpy(&_info,_in->getInfo(),sizeof(_info));
}

ADMVideoPartial::ADMVideoPartial(  AVDMGenericVideoStream *in,VF_FILTERS tag,CONFcouple *setup)
{
		_param=NEW( PARTIAL_CONFIG);
		_param->_start=0;
		_param->_end=0;
		_param->_tag=tag;
		_in=in;
		// we share the same parameters
		_son= filterCreateFromTag(tag,setup,_in);
		memcpy(&_info,_in->getInfo(),sizeof(_info));
}

/**/
uint8_t	ADMVideoPartial::getCoupledConf( CONFcouple **couples)
{
			uint32_t nbParam=0;

			ADM_assert(_param);

			// first we ask the child its config
			CONFcouple *child;
			if(!_son->getCoupledConf(&child))
			{
				// no param for child
#define CSET(x)  (*couples)->setCouple((char *)#x,(_param->x))
				*couples=new CONFcouple(3);
				CSET(_start);
				CSET(_end);
				CSET(_tag);
				return 1;
			}

			// grab child param
			nbParam=child->getNumber();

			*couples=new CONFcouple(3+nbParam);
			CSET(_start);
			CSET(_end);
			CSET(_tag);

	// then set the child ones
	char *nm,*vl;

	for(uint32_t i=0;i<nbParam;i++)
	{
		child->getEntry(i,&nm, &vl);
		  (*couples)->setCouple(nm,vl);
	}
	// delete child
	delete child;

	return 1;

}

//
ADMVideoPartial::~ADMVideoPartial()
{
		if(_son) delete _son;
		DELETE(_param);

}
//
uint8_t ADMVideoPartial::getFrameNumberNoAlloc(uint32_t frame,
				uint32_t *len,
   				ADMImage *data,
				uint32_t *flags)
{
		ADM_assert(_son);
		if(frame>= _info.nb_frames) return 0;
		if((frame+_info.orgFrame>=_param->_start) && (frame+_info.orgFrame)<=_param->_end) //
		{
				return _son->getFrameNumberNoAlloc(frame,len,data,flags);
		}
		return _in->getFrameNumberNoAlloc(frame,len,data,flags);
}
//
extern uint8_t DIA_getPartial(PARTIAL_CONFIG *param,AVDMGenericVideoStream *son,AVDMGenericVideoStream *previous);
uint8_t ADMVideoPartial::configure( AVDMGenericVideoStream *instream)
{
  uint8_t r=0;
    r=DIA_getPartial(_param,_son,_in);
    if(_param->_end<_param->_start) _param->_end=_param->_start;

   return r;
}




