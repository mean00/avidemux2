/** *************************************************************************
        \file ADM_confCouple.cpp
        \brief Handle storage of data into key/value pairs

    copyright            : (C) 2002/2009 by mean
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
#include "ADM_confCouple.h"

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
/**
    \fn exist
*/
bool CONFcouple::exist(const char *name)
{
    int i=lookupName(name);
    if(i==-1) return false;
    return true;
}
bool CONFcouple::writeAsUint32(const char *myname,uint32_t val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%"LU,val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
bool CONFcouple::writeAsFloat(const char *myname,float val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%f",val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
bool CONFcouple::writeAsInt32(const char *myname,int32_t val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	sprintf(tmpstring,"%"LD,val);
	value[cur]=ADM_strdup(tmpstring);
	cur++;
	return 1;
}
#warning TODO: ESCAPE!
bool CONFcouple::writeAsString(const char *myname,const char *val)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
	value[cur]=ADM_strdup(val);
	cur++;
	return 1;
}
bool CONFcouple::writeAsBool(const char *myname,bool v)
{
	ADM_assert(cur<nb);

	name[cur]=ADM_strdup(myname);
    if(v==true) value[cur]=ADM_strdup("True");
        else value[cur]=ADM_strdup("False");
	
	cur++;
	return 1;
}

// ******************************************
bool CONFcouple::readAsBool(const char *myname,bool *v)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
    char *test=value[index];
    if(!strcasecmp(test,"true")) *v=true;
        else *v=false;
	return 1;
}
bool CONFcouple::readAsUint32(const char *myname,uint32_t *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=(int)atoi(value[index]);
	return 1;
}
bool CONFcouple::readAsInt32(const char *myname,int32_t *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=(int)atoi(value[index]);
	return 1;
}
bool CONFcouple::readAsString(const char *myname,char **val)
{
	int32_t index=lookupName(myname);
#warning TODO : unescape
	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	*val=ADM_strdup(value[index]);
	return 1;
}

bool CONFcouple::readAsFloat(const char *myname,float *val)
{
	int32_t index=lookupName(myname);

	ADM_assert(index!=-1);
	ADM_assert(index<(int)nb);
	sscanf(value[index],"%f",val);;
	return 1;
}
/**
    \fn lookupName
    \brief Return index of name in the couples, -1 if not fount
*/
int32_t CONFcouple::lookupName(const char *myname)
{
	for(uint32_t i=0;i<nb;i++)
	{
		if(!strcmp(name[i],myname)) return i;

	}
	return -1;

}
/**
    \fn duplicate
    \brief clone a conf couple
*/
CONFcouple *CONFcouple::duplicate(CONFcouple *source)
{
    if(!source) return NULL;
    int nb=source->nb;
    CONFcouple *nw=new CONFcouple(nb);
    for(int i=0;i<nb;i++)
    {
            char  *n,*v;
        	source->getInternalName(i,&n,&v);
            nw->setInternalName(n,v);
    }
    return nw;
}
/**
    \fn dump
    \brief dump
*/
void CONFcouple::dump(void )
{
	for(uint32_t i=0;i<nb;i++)
	{
		if(name[i]) printf ("nm:%s ",name[i]); else printf("!! no name !! ");
		if(value[i]) printf ("val:%s ",value[i]); else printf("!! no value !! ");
	}
}

 bool     CONFcouple::setInternalName(const char *nm, const char *val)
{
   ADM_assert(cur<nb);

	name[cur]=ADM_strdup(nm);
	value[cur]=ADM_strdup(val);
	cur++;
	return 1;
}
bool  CONFcouple::getInternalName(uint32_t n, char **nm, char **val)
{ 
    assert(n<nb); 
    *nm=name[n];
    *val=value[n];
    return true;
};

/**
    \fn stringsToConfCouple
    \brief Convert js args to confcouple

*/
bool stringsToConfCouple(int nb,CONFcouple **conf,  const char **argv)
{
  *conf=NULL;
  if(!nb) return true;
  CONFcouple *c=new CONFcouple(nb);
  *conf=c;
    for(int i=0;i<nb;i++)
    {
        
        char *dupe=   ADM_strdup(argv[i]);
        char *name,*value;
        // dupe is in the form name=value
        name=dupe;
        value=name;
        char *tail=dupe+strlen(dupe);
        while(value<tail)
        {
            if(*value=='=') 
                {
                    *value=0;
                    value++;
                    break;
                }
            value++;
        }
        c->setInternalName(name,value);
        //printf("%s -> [%s,%s]\n",param,name,value);
        ADM_dezalloc(dupe);
    }
    return true;
}
