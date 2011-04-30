/***************************************************************************
   
    \file  prefs.cpp
    \brief 
    copyright            : (C) 2001 by mean
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


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "ADM_cpp.h"
#include "ADM_default.h"
#include "ADM_quota.h"
#include "ADM_paramList.h" 
#include "prefs.h"

#include "prefs2.h"
#include "prefs2_desc.cpp"

#define CONFIG "config2"
#define FILE_SIZE_MAX (20*1024)
extern char *ADM_getBaseDir(void);
static char *checkDirAccess(char *home);
extern char *ADM_escape(const ADM_filename *incoming);
class preferences *prefs;
static my_prefs_struct myPrefs;

bool  my_prefs_struct_jserialize(const char *file, const my_prefs_struct *key);
bool  my_prefs_struct_jdeserialize(const char *file, const ADM_paramList *tmpl,my_prefs_struct *key);

typedef struct
{
   const char *name;
   const char *name2;
   ADM_paramType  type;
   const char *defaultValue;
   const char *min;
   const char *max;
   char *Value;
}optionDesc;

#include "prefs2_pref.h"

/**
    \fn initPrefs
*/
int initPrefs(  void )
{
  prefs = new preferences();
  return 1;
}
/**
    \fn destroyPrefs
*/
int destroyPrefs(void)
{
	delete prefs;
	prefs = NULL;
	return 1;
}

static int searchOptionByName2(const char *name)
{
    int nb=sizeof( myOptions)/sizeof(optionDesc);
    for(int i=0;i<nb;i++)
        if(!strcmp(myOptions[i].name2,name)) return i;
    return -1;
}
/**
    \fn ctor
*/
preferences::preferences()
{
	internal_lastfiles[0] = internal_lastfiles[1] = NULL;
	internal_lastfiles[2] = internal_lastfiles[3] = NULL;
	internal_lastfiles[4] = NULL;
    // set default...
    int nb=sizeof( my_prefs_struct_param)/sizeof(ADM_paramList);
    for(int i=0;i<nb-1;i++) //
    {
            char *dummyPointer=(char *)&myPrefs;

            const ADM_paramList *param=my_prefs_struct_param+i;
            int offset=param->offset;
            const char *name=param->paramName;

            int rank=searchOptionByName2(name);
            ADM_assert(rank!=-1);
            const optionDesc *opt=myOptions+rank;
            ADM_assert(myOptions[rank].type==param->type);
            
            switch(param->type)
            {
                case ADM_param_uint32_t:
                                    *(uint32_t *)(dummyPointer+offset)=atoi(opt->defaultValue);
                                    break;
                case ADM_param_int32_t:
                                    *(int32_t *)(dummyPointer+offset)=atoi(opt->defaultValue);
                                    break;
                case ADM_param_float:
                                    *(float *)(dummyPointer+offset)=atof(opt->defaultValue);
                                    break;
                case ADM_param_bool:
                                    *(bool *)(dummyPointer+offset)=atoi(opt->defaultValue);
                                    break;
                case ADM_param_string:
                                    {
                                        char **z=(char **)(dummyPointer+offset);
                                        *z=ADM_strdup(opt->defaultValue);
                                    }
                                    break;
                default:
                        ADM_error("Type not authorized for prefs %s\n",name);
                        ADM_assert(0);

            }
            // 
    
    }
}

preferences::~preferences(){
  unsigned int idx;
	for( idx=0; idx < 4; idx++ ){
		if( internal_lastfiles[idx] )
			ADM_dealloc(internal_lastfiles[idx]);
	}
	
	
}



/**
    \fn load
    \brief load prefs from file.. Should be called only once
*/
int preferences::load()
{

   char *home;
   char *dir_adm;
   std::string path;


    dir_adm=ADM_getBaseDir();
    if(!dir_adm) return RC_FAILED;

    path=string(dir_adm);
    path=path+std::string("/");
    path=path+std::string(CONFIG);
    ADM_info("Loading prefs from %s\n",path.c_str());
    // exist ?
    if(!ADM_fileExist(path.c_str()))
    {
		ADM_error("can't read %s\n",			path.c_str());
		return RC_FAILED;
    }
    if(true==my_prefs_struct_jdeserialize(path.c_str(),my_prefs_struct_param,&myPrefs))
    {
        ADM_info("Preferences found and loaded\n");
        return RC_OK;
    }
    ADM_warning("An error happened while loading config\n");
    return RC_FAILED;
}

/**
    \fn save
*/
int preferences::save()
{
   char *home;
   char *dir_adm;
   std::string path;


    dir_adm=ADM_getBaseDir();
    if(!dir_adm) return RC_FAILED;

    path=string(dir_adm);
    path=path+std::string("/");
    path=path+std::string(CONFIG);
    string tmp=path;
    tmp=tmp+string(".tmp");
    ADM_error("Saving prefs to %s\n",tmp.c_str());

   if(true==my_prefs_struct_jserialize(tmp.c_str(),&myPrefs))
    {
        ADM_copyFile(tmp.c_str(),path.c_str());
        ADM_eraseFile(tmp.c_str());
        return RC_OK;
    }
    ADM_error("Cannot save prefs\n");
    return RC_FAILED;
}

//--------
int preferences::get(options option, uint8_t *val)
{
	return RC_FAILED;
}


int preferences::get(options option, uint16_t *val)
{
	return RC_FAILED;
}

int preferences::get(options option, unsigned int *val)
{
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option,          int *val)
{

	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, unsigned long *val)
{

	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, long *val)
{
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, float *val)
{
	return RC_FAILED; // wrong input for conversion or EOF
}

int preferences::get(options option, char **val)
{
	return RC_FAILED; // strdup() out of memory
}

int preferences::set(options option, const unsigned int val)
{
	return RC_OK;
}

int preferences::set(options option, const int val)
{
	return RC_OK;
}

int preferences::set(options option, const unsigned long val)
{
	return RC_OK;
}

int preferences::set(options option, const long val)
{
	return RC_OK;
}

int preferences::set(options option, const float val)
{
	return RC_OK;
}

int preferences::set(options option, const char * val)
{
	return RC_OK;
}

#define PRT_LAFI(x,y,z) fprintf(stderr,"Prefs: %s%u %s\n",x,y,(z?z:"NULL"))

int preferences::set_lastfile(const char* file)
{

	return RC_OK;
}

#undef PRT_LAFI
#define PRT_LAFI(y,z) fprintf(stderr,"Prefs: ret idx[%u] %s\n",y,(z?z:"NULL"))

const char **preferences::get_lastfiles(void)
{

	return (const char**)internal_lastfiles;
}

// EOF
