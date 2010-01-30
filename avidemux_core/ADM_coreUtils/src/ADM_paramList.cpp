/** *************************************************************************
    \file ADM_paramList
    \brief Handle Param list 
                      
    copyright            : (C) 2009 by mean
    
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
#include "ADM_coreVideoEncoderFFmpeg_param.h"

static bool lavReadFromString(FFcodecContext *ctx,const char *str)
{
    printf("String:%s\n",str);
    return true;
}
static bool lavWriteToString(FFcodecContext *ctx,char **str)
{
    *str=ADM_strdup("foobar");
    return true;
}

/**
    \fn ADM_paramValidate
    \brief Check the confcouples match the param list
*/
bool ADM_paramValidate(CONFcouple *couples, const ADM_paramList *params)
{
    int n=couples->getSize();
    int found=0;
    int p=0;
    const ADM_paramList *l=params;
    while(l->paramName)
    {
        p++;
        l++;
    };
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
    \fn ADM_paramLoad
    \brief Load a structure from a list of confCouple
*/
bool ADM_paramLoad(CONFcouple *couples, const ADM_paramList *params,void *s)
{
    uint8_t *address=(uint8_t *)s;
    if(false==ADM_paramValidate(couples,params)) return false;
    int n=couples->getSize();
    for(int i=0;i<n;i++)
    {
        const char *name=params[i].paramName;
        int index=couples->lookupName(name);
        ADM_assert(index!=-1);
        switch(params[i].type)
        {
#define SWAL(entry,type,var,access) case  entry: {type   var;\
                        couples->readAs##access(name,&var); \
                        *(type *)(address+params[i].offset)=var;}break;
           SWAL(ADM_param_uint32_t,uint32_t,u32,Uint32)
           SWAL(ADM_param_int32_t, int32_t, i32,Int32)
           SWAL(ADM_param_float,   float ,  f,Float)
           SWAL(ADM_param_bool,    bool ,   b,Bool)
           case ADM_param_lavcodec_context:
                        {
                        char *lavString;
                        if(false==couples->readAsString(name,&lavString))
                        {
                                ADM_error("Error reading lavcodec conf");
                                return false;
                        }
                        bool r=lavReadFromString((FFcodecContext *)(address+params[i].offset),lavString);
                        ADM_dezalloc(lavString);
                        if(false==r)
                            {
                                    ADM_error("Error reading lavcodec string");
                                    return false;
                            }
                       }
                        break;
                        
           case ADM_param_string: ADM_error("not implemented string for paramList\n");ADM_assert(0);
        }
    }
    return true;
}
/**
    \fn ADM_paramLoad
    \brief Load a structure from a list of confCouple
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
           SWAL(ADM_param_bool,    bool ,   b,Bool)
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
           case ADM_param_string: ADM_error("not implemented string for paramList\n");ADM_assert(0);
        }
    }
    return true;
}
