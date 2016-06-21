/** *************************************************************************
    \file ADM_paramList
    \brief Handle Param list
    \author  mean (C) 2009/2010   fixounet@free.fr
    Usually only basic types should be handled by this
    We make 2 exceptions though :
        * lavcodec settings as we use a lot of derivative class. It is to simplify derivative configuration
        * same for COMPRESS_PARAMS, since is used by all video encoder, it is "smart" to make it a dedicated
                    configuration type.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_paramList.h"
#include <stddef.h>
#include "ADM_encoderConf.h"
#include "ADM_coreVideoEncoderFFmpeg_param.h"
#include "FFcodecContext_param.h"
#include "../src/FFcodecContext_desc.cpp"
#define MAX_LAV_STRING 1024
/**
    \fn getCoupleFromString
*/
void getCoupleFromString(CONFcouple **couples, const char *str,const ADM_paramList *tmpl)
{
  // Split str to couples...
    uint32_t nb=0;
    const char *s=str;
    char tmp[256];
    while(*s)
    {
        if(*s==':') nb++;
        s++;
    }

    int p=0;
    const ADM_paramList *l=tmpl;
    while(l->paramName)
    {
        p++;
        l++;
    };
    if(nb!=p)
    {
        ADM_error("Mistmatch in the number of parameters (%d/%d)\n",(int)nb,(int)p);
        *couples = NULL;
        return;
    }
    *couples=new CONFcouple(nb);
    s=str;
    const char *n;
    for(int i=0;i<nb;i++)
    {
        if(*s!=':')
        {
            ADM_error("Bad split :%s instead of ':'\n",s);
            delete [] couples;
            *couples = NULL;
            return;
        }
        n=s+1;
        while(*n!=':' && *n) n++;
        n--;
        memcpy(tmp,s+1,n-s);
        tmp[n-s]=0;
        s=n+1;
       // printf("tmp:%s\n",tmp);
        // Now we have aaa=bbb in tmp, split it
        char *equal,*tail;
        equal=tmp;
        tail=tmp+strlen(tmp);
        while(*equal!='=' && equal<tail)
        {
            equal++;
        }
        if(*equal!='=')
        {
            ADM_error("Malformed string :%s\n",tmp);
            delete [] couples;
            *couples = NULL;
            return;
        }
        *equal=0;
        (*couples)->setInternalName(tmp,equal+1);
       // printf("%s->%s\n",tmp,equal+1);
    }
}

void lavCoupleToString(CONFcouple *couples, char **str)
{
	char *s = (char *)ADM_alloc(MAX_LAV_STRING);
	char tmp[256];
	*s = 0;
	*str = s;
	uint32_t nb = couples->getSize();

	for (int i = 0; i < nb; i++)
	{
		char *name, *value;
		couples->getInternalName(i, &name, &value);
		sprintf(tmp, ":%s=%s", name, value);
		ADM_assert(strlen(tmp) < 255);
		strcat(s, tmp);
		ADM_assert(strlen(s) < MAX_LAV_STRING);
	}
}

/**
    \fn lavReadFromString
*/
static bool lavReadFromString(FFcodecContext *ctx, const char *str)
{
	CONFcouple* couples;
	bool success = false;

	getCoupleFromString(&couples, str, FFcodecContext_param);

	if (couples != NULL)
	{
		success = ADM_paramLoad(couples, FFcodecContext_param, ctx);
	}

	delete couples;

	return success;
}
/**
    \fn lavWriteToString
*/
static bool lavWriteToString(FFcodecContext *ctx, char **str)
{
	CONFcouple *couples = NULL;

	if (false == ADM_paramSave(&couples, FFcodecContext_param, ctx))
	{
		ADM_error("ADM_paramSave failed (lavContext)\n");
		return false;
	}

	lavCoupleToString(couples, str);

	delete couples;
	return true;
}
/**
    \fn compressReadFromString
*/
static bool compressReadFromString(COMPRES_PARAMS *params,const char *str)
{
    char tmp[256];
    if(!strcasecmp(str,"SAME"))
    {
        params->mode=COMPRESS_SAME;
        return true;
    }
    // all other are in the form a=b
    strcpy(tmp,str);
    char *s=tmp;
    while(*s)
    {
        if(*s=='=') break;
        s++;
    }
    if(!(*s))
    {
        ADM_error("Malformed compressVideo line (%s)\n",str);
        return false;
    }
    *s=0;
    uint32_t val=atoi(s+1);
    if(!strcasecmp(tmp,"CQ"))    {params->mode=COMPRESS_CQ;params->qz=val;return true;}
    if(!strcasecmp(tmp,"CBR"))   {params->mode=COMPRESS_CBR;params->bitrate=val;return true;}
    if(!strcasecmp(tmp,"2PASS")) {params->mode=COMPRESS_2PASS;params->finalsize=val;return true;}
    if(!strcasecmp(tmp,"2PASSBITRATE")) {params->mode=COMPRESS_2PASS_BITRATE;params->avg_bitrate=val;return true;}
    if(!strcasecmp(tmp,"AQ")) {params->mode=COMPRESS_AQ;params->qz=val;return true;}
    ADM_error("Unknown mode :%s\n",tmp);
    return false;

}
/**
    \fn ADM_compressWriteToString
*/
bool ADM_compressWriteToString(COMPRES_PARAMS *params,  char **str)
{
    char tmp[256];
    switch(params->mode)
    {
        case COMPRESS_CQ:    sprintf(tmp,"CQ=%" PRIu32,params->qz);break;
        case COMPRESS_CBR:   sprintf(tmp,"CBR=%" PRIu32,params->bitrate);break;
        case COMPRESS_2PASS: sprintf(tmp,"2PASS=%" PRIu32,params->finalsize);break;
        case COMPRESS_SAME:  sprintf(tmp,"SAME");break;
        case COMPRESS_2PASS_BITRATE: sprintf(tmp,"2PASSBITRATE=%" PRIu32,params->avg_bitrate);break;
        case COMPRESS_AQ:    sprintf(tmp,"AQ=%" PRIu32,params->qz);break;
        default:
            ADM_error("Unknown compressin mode \n");
            return false;
    }
    *str=ADM_strdup(tmp);
    return true;
}
/**
    \fn getParamSize
*/
static int XgetParamSize( const ADM_paramList *params)
{
    int p=0;
    const ADM_paramList *l=params;
    while(l->paramName)
    {
        p++;
        l++;
    };
    return p;
}
/**
    \fn ADM_paramValidate
    \brief Check the confcouples match the param list
*/
bool ADM_paramValidate(CONFcouple *couples, const    ADM_paramList *params)
{
    int n=couples->getSize();
    int p=XgetParamSize(params);
    if(n!=p)
    {
        ADM_warning("Number of parameter mistmatch :%d vs %d\n",n,p);
        return false;
    }
    // Now for each param, check we have it...
    for(int i=0;i<p;i++)
    {
        const char *name=params[i].paramName;
        if(false==couples->exist(name))
        {
            ADM_warning("Cannot find param with name %s in configuration\n",name);
            return false;
        }
    }
    return true;
}
/**
    \fn ADM_paramValidatePartialList
    \brief In that case couples is a sublist of params
*/
bool ADM_paramValidatePartialList(CONFcouple *couples, const    ADM_paramList *params)
{
    int n=couples->getSize();
    int p=XgetParamSize(params);
    if(n>p)
    {
        ADM_warning("Too many parameters in partial list");
        return false;
    }
    int found=0;
    for(int i=0;i<p;i++)
    {
        const char *name=params[i].paramName;
        if(true==couples->exist(name))
        {
            found++;
        }else
        {
                ADM_warning("\tParam : <%s> not found\n",name);
        }
    }
    if(found==n) return true;
    ADM_warning("Some parameters are not in the parameter list, typo ?(%d vs %d)\n",(int)found,(int)n);
    return false;;
}

/**
    \fn ADM_paramLoadInternal
    \brief Load a structure from a list of confCouple
*/
static bool ADM_paramLoadInternal(bool partial,CONFcouple *couples, const ADM_paramList *params,void *s)
{
    uint8_t *address=(uint8_t *)s;
    int p=XgetParamSize(params);

    for(int i=0;i<p;i++)
    {
        const char *name=params[i].paramName;
        int index=couples->lookupName(name);
        if(index==-1) // not found ?
        {
            if(false==partial)
            {
                ADM_assert(index!=-1); // Should be replaced later by a return false
            }else
            {
               // ADM_info("%s not found\n",name);
                continue; // this parameter is not in the param list and we are doing a partial update
            }
        }
        switch(params[i].type)
        {
#define SWAL(entry,type,var,access) case  entry: {type   var;\
                        couples->readAs##access(name,&var); \
                        *(type *)(address+params[i].offset)=var;};break;
           SWAL(ADM_param_uint32_t,uint32_t,u32,Uint32)
           SWAL(ADM_param_int32_t, int32_t, i32,Int32)
           SWAL(ADM_param_float,   float ,  f,Float)
	   SWAL(ADM_param_double,  double,  d,Double)
           SWAL(ADM_param_bool,    bool ,   b,Bool)
           case ADM_param_video_encode:
                        {
                        char *lavString;
                        if(false==couples->readAsString(name,&lavString))
                        {
                                ADM_error("Error reading video_encode conf");
                                return false;
                        }
                        bool r=compressReadFromString((COMPRES_PARAMS *)(address+params[i].offset),lavString);
                        delete [] lavString;
                        if(false==r)
                            {
                                    ADM_error("Error reading codecParam string");
                                    return false;
                            }
                        }
                        break;
           case ADM_param_lavcodec_context:
                        {
                        char *lavString;
                        if(false==couples->readAsString(name,&lavString))
                        {
                                ADM_error("Error reading lavcodec conf");
                                return false;
                        }
                        bool r=lavReadFromString((FFcodecContext *)(address+params[i].offset),lavString);
                        delete [] lavString;
                        if(false==r)
                            {
                                    ADM_error("Error reading lavcodec string");
                                    return false;
                            }
                       }
                        break;

           case ADM_param_string:
                    {
                        char   *var;
                        couples->readAsString(name,&var);
                        char *clone=ADM_strdup(var); // parm want alloc/free but ConfCouple create new [] / Delete []
                        *(char  **)(address+params[i].offset)=clone;
                        delete [] var;
                    }
                    break;
            default:
                    ADM_error("Type no handled %d\n",params[i].type);
                    break;
        }
    }
    return true;
}
/**
    \fn ADM_paramLoad
    \brief Load a structure from a list of confCouple, validate that all fields are there
*/
bool ADM_paramLoad(CONFcouple *couples, const ADM_paramList *params,void *s)
{
    if(!couples && !params)
    {
          ADM_warning("Empty parameter list\n");
          return true;
    }
    if(false==ADM_paramValidate(couples,params)) return false;
    return ADM_paramLoadInternal(false,couples,params,s);

}
/**
    \fn ADM_paramLoad
    \brief Load a structure from a list of confCouple, accept partial fields
*/

bool ADM_paramLoadPartial(CONFcouple *couples, const ADM_paramList *params,void *s)
{
   if(false==ADM_paramValidatePartialList(couples,params)) return false;
   return ADM_paramLoadInternal(true,couples,params,s);
}
/**
    \fn ADM_paramLoad
    \brief Save a structure to a list of confCouple
*/
bool ADM_paramSave(CONFcouple **couples, const ADM_paramList *params,void *s)
{
    *couples=NULL;
    int p=0;
    const ADM_paramList *l=params;
    while(l->paramName)
    {
        p++;
        l++;
    };
    *couples=new CONFcouple(p);
    CONFcouple *c=*couples;
    uint8_t *address=(uint8_t *)s;

    for(int i=0;i<p;i++)
    {
        const char *name=params[i].paramName;
        switch(params[i].type)
        {
#undef SWAL
#define SWAL(entry,type,var,access) case  entry: \
                                    {type var; \
                                    var=*(type *)(address+params[i].offset);\
                                    c->writeAs##access(name,var);}break;
           SWAL(ADM_param_uint32_t,uint32_t,u32,Uint32)
           SWAL(ADM_param_int32_t, int32_t, i32,Int32)
           SWAL(ADM_param_float,   float ,  f,Float)
           SWAL(ADM_param_double,  double,  d,Double)
           SWAL(ADM_param_bool,    bool ,   b,Bool)
            case ADM_param_video_encode:
              {
                        char *lavString;
                        if(false==ADM_compressWriteToString((COMPRES_PARAMS *)(address+params[i].offset),&lavString))
                        {
                                ADM_error("Error writing paramvideo string");
                                return false;
                        }
                        bool r=c->setInternalName(name,lavString);
                        ADM_dezalloc(lavString);
                        if(false==r)
                        {
                                ADM_error("Error writing paramvideo conf");
                                return false;
                        }
              }
                break;
           case ADM_param_lavcodec_context:
              {
                        char *lavString;
                        if(false==lavWriteToString((FFcodecContext *)(address+params[i].offset),&lavString))
                        {
                                ADM_error("Error writing lavcodec string");
                                return false;
                        }
                        bool r=c->setInternalName(name,lavString);
                        ADM_dezalloc(lavString);
                        if(false==r)
                        {
                                ADM_error("Error writing lavcodec conf");
                                return false;
                        }
              }
                break;
           case ADM_param_string:
                {
                        char *var;
                        var=*(char  **)(address+params[i].offset);
                        bool r=c->writeAsString(name,var);
                        if(false==r)
                        {
                                ADM_error("Error writing string\n");
                                return false;
                        }
                }
                break;
            default:
                    ADM_assert(0);
        }
    }
    return true;
}
