/***************************************************************************
                          ADM_genvideo.cpp  -  description
                             -------------------
    begin                : Sun Apr 14 2002
    copyright            : (C) 2002 by mean
    email                : fixounet@free.fr
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

//#include "fourcc.h"

#include "ADM_confCouple.h"
//#include "ADM_videoFilter.h"
static char tmpstring[1024]; // should be enougth
CONFcouple::CONFcouple(uint32_t nub)
{
	nb=nub;
	name=new char *[nb];
	value=new char *[nb];

	for(uint32_t i=0;i<nb;i++)
		{
			name[i]=NULL;
			value[i]=NULL;
		}

	cur=0;
};

CONFcouple::~CONFcouple()
{
	for(uint32_t i=0;i<nb;i++)
		{
			if(name[i]) delete name[i];
			if(value[i]) delete value[i];
		}
		delete [] name;
		delete [] value;


};

uint8_t CONFcouple::setCouple(const char *myname,uint32_t val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%"LU,val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
uint8_t CONFcouple::setCouple(const char *myname,float val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%f",val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
uint8_t CONFcouple::setCouple(const char *myname,double val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%f",val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
uint8_t CONFcouple::setCouple(const char *myname,int32_t val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%"LD,val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
uint8_t CONFcouple::setCouple(const char *myname,const char *val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	value[cur]=ADM_strdup(val);
	cur++;
	return 1;
}
uint8_t CONFcouple::setCouple(const char *myname,const ADM_filename *val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	value[cur]=ADM_strdup((char *)val);
	cur++;
	return 1;
}

uint8_t CONFcouple::getCouple(const char *myname,uint32_t *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=(int)atoi(value[index]);
	return 1;
}
uint8_t CONFcouple::getCouple(const char *myname,int32_t *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=(int)atoi(value[index]);
	return 1;
}
uint8_t CONFcouple::getCouple(const char *myname,char **val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=ADM_strdup(value[index]);
	return 1;
}
uint8_t CONFcouple::getCouple(const char *myname,ADM_filename **val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=(ADM_filename *)ADM_strdup(value[index]);
	return 1;
}

uint8_t CONFcouple::getCouple(const char *myname,float *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	sscanf(value[index],"%f",val);;
	return 1;
}
uint8_t CONFcouple::getCouple(const char *myname,double *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	sscanf(value[index],"%lf",val);;
	return 1;
}

int32_t CONFcouple::lookupName(const char *myname)
{
	for(uint32_t i=0;i<nb;i++)
	{
		if(!strcmp(name[i],myname)) return i;

	}
	return -1;

}
void CONFcouple::dump(void )
{
	for(uint32_t i=0;i<nb;i++)
	{
		if(name[i]) printf ("nm:%s ",name[i]); else printf("!! no name !! ");
		if(value[i]) printf ("val:%s ",value[i]); else printf("!! no value !! ");
	}
}

