/***************************************************************************
                          \fn ADM_coreVideoEncoder
                          \brief Base class for video encoder plugin
                             -------------------

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

#include <sstream>

#include "ADM_default.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_coreVideoEncoderInternal.h"
#include "ADM_vidMisc.h"
#include "BVector.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

#ifdef _WIN32
const std::string slash=std::string("\\");
#else
const std::string slash=std::string("/");
#endif

#define MAX_NB_PRESET 30

BVector <ADM_videoEncoder6 *> ListOfEncoders;

/**
    \fn ADM_coreVideoEncoder
*/
ADM_coreVideoEncoder::ADM_coreVideoEncoder(ADM_coreVideoFilter *src)
{
    source=src;
    image=NULL;
    encoderDelay=0;
    lastDts=ADM_NO_PTS;
}

/**
    \fn ADM_coreVideoEncoder
*/
ADM_coreVideoEncoder::~ADM_coreVideoEncoder()
{
    if(image) delete image;
    image=NULL;
}
typedef struct
{
    uint64_t mn,mx;
    int n,d;

}TimeIncrementType;

TimeIncrementType fpsTable[]=
{
    {  40000,40000,1 ,25},
    {  20000,20000,1 ,50},
    {  33360,33371,1001,30000},
    {  41700,41710,1001,24000},
};

/**
    \fn usSecondsToFrac
    \brief Convert a duration in useconds into Rationnal
*/
bool usSecondsToFrac(uint64_t useconds, int *n,int *d)
{
    // First search for known value...
    int nb=sizeof(fpsTable)/sizeof(TimeIncrementType);
    for(int i=0;i<nb;i++)
    {
        TimeIncrementType *t=fpsTable+i;
        if( useconds>=t->mn && useconds<=t->mx)
        {
            *n=t->n;
            *d=t->d;
            return true;
        }
    }
    int nn,dd;
    av_reduce(&nn,&dd, useconds, 1000000, 0xFFF0); // mpeg4 allows a maximum of 1<<16-1 as time base, should be enough for most case
    ADM_info("%"LLU" us -> %d / %d (old)\n",useconds,nn,dd);
    *n=nn;
    *d=dd;

    return true;
}

/**
    \fn getRealPtsFromInternal
    \brief Lookup in the stored value to get the exact pts from the truncated one
*/
bool ADM_coreVideoEncoder::getRealPtsFromInternal(uint64_t val,uint64_t *dts,uint64_t *pts)
{
    int n=mapper.size();
    for(int i=0;i<n;i++)
    {
        if(mapper[i].internalTS==val)
        {
            *pts=mapper[i].realTS;
            mapper.erase(mapper.begin()+i);
            // Now get DTS, it is min (lastDTS+inc, PTS-delay)
            ADM_assert(queueOfDts.size());
            *dts=queueOfDts[0];
            queueOfDts.erase(queueOfDts.begin());
            if(*dts>*pts)
            {
                ADM_warning("Dts>Pts, that can happen if there are holes in the source, fixating..\n");
                ADM_warning("DTS=%s\n",ADM_us2plain(*dts));
                ADM_warning("PTS=%s\n",ADM_us2plain(*pts));
                if(lastDts!=ADM_NO_PTS)
                {
                    uint64_t newDts=lastDts+getFrameIncrement();
                    if(newDts<=*pts)
                    {
                            ADM_warning("Using newDts=%"LLU"\n",newDts);
                            *dts=newDts;
                            return true;
                    }
                }
                ADM_error("Cannot find a solution, expect problems\n");
                *dts=*pts;
            }
            return true;
        }
    }
    ADM_warning("Cannot find PTS : %"LLU"\n",val);
    for(int i=0;i<n;i++) ADM_warning("%d : %"LLU"\n",i,mapper[i].internalTS);
    ADM_assert(0);
    return false;

}
/**
    \fn ADM_pluginSystemPath
*/
static bool ADM_pluginSystemPath(const std::string& pluginName,int pluginVersion,std::string &rootPath)
{

    std::string path=std::string(ADM_getSystemPluginSettingsDir());
    std::string version;
    std::stringstream out;
    out << pluginVersion;
    version=out.str();
    path=path+slash+std::string(pluginName);
    path=path+slash+version;
    rootPath=path;
    ADM_info("System Plugin preset path : %s\n",rootPath.c_str());
    return true;
}
/**
    \fn ADM_pluginGetPath
    \brief returns the user plugin path, containing the presets for that plugin
*/
bool ADM_pluginGetPath(const std::string& pluginName,int pluginVersion,std::string &rootPath)
{
    std::string path=std::string(ADM_getUserPluginSettingsDir());
    std::string version;
    std::stringstream out;
    out << pluginVersion;
    version=out.str();
    ADM_mkdir(path.c_str());
    path=path+slash+std::string(pluginName);
    ADM_mkdir(path.c_str());
    path=path+slash+version;
    ADM_mkdir(path.c_str());
    rootPath=path;
    ADM_info("Plugin preset path : %s\n",rootPath.c_str());
    return true;
}
/**
    \fn getFileNameAndExt
    \brief only keep filename and ext from input (i.e. remove folders)
*/
static bool getFileNameAndExt(const std::string &input, std::string &output)
{
    std::string s=input;
    size_t lastSlash;

     lastSlash=s.find_last_of("/");
     if(lastSlash!=-1)
        s.replace(0,lastSlash+1,std::string(""));
#ifdef _WIN32
     size_t lastBackSlash=s.find_last_of("\\");
     if(lastBackSlash!=-1)
        s.replace(0,lastBackSlash+1,std::string(""));
#endif
    ADM_info("Stripping : %s => %s\n",input.c_str(),s.c_str());
    output=s;
    return true;
}

/**
    \fn ADM_pluginInstallSystem
    \brief Copy if needed the system presets to the user presets list
*/
bool ADM_pluginInstallSystem(const std::string& pluginName,const std::string& ext,int pluginVersion)
{
    std::string sysPath,userPath;
    ADM_pluginSystemPath(pluginName,pluginVersion,sysPath);
    ADM_pluginGetPath(pluginName,pluginVersion,userPath);
    char *list[MAX_NB_PRESET];
    uint32_t nb=0;
    ADM_info("Looking for file %s in folder %s\n",ext.c_str(),sysPath.c_str());
    if(false==buildDirectoryContent(&nb,sysPath.c_str(),list,MAX_NB_PRESET,ext.c_str()))
    {
        ADM_info("No preset found\n");
        return true;
    }
    ADM_info("Found %d files\n",(int)nb);
    for( int i=0;i<nb;i++)
    {
        std::string s=std::string(list[i]);
        std::string file;
        getFileNameAndExt(s,file);
        s=userPath+slash+file; //
        if(!ADM_fileExist(s.c_str()))
        {
            ADM_info("%s exists in system folder, but not in user folder, copying..\n",file.c_str());
            ADM_copyFile(list[i],s.c_str());
        }
    }
    clearDirectoryContent(nb,list);
    return true;
}
/**
    \fn ADM_pluginGetPath
    \brief returns the user plugin path, containing the presets for that plugin
*/
bool ADM_listFile(const std::string& path,const std::string& extension,vector <std::string > & listOut)
{
#define NB 30
    char *list[NB];
    uint32_t nb=0;

    listOut.clear();
    if(false==buildDirectoryContent(&nb,path.c_str(),list,NB,extension.c_str()))
    {
        ADM_info("No preset found\n");
        return true;
    }
    size_t lastDot;
    for(int i=0;i<nb;i++)
    {
        std::string s=std::string(list[i]);
        std::string file;
        getFileNameAndExt(s,file);
        lastDot=file.find_last_of('.');
        // Remove extension
        if(lastDot!=-1)
            file.replace(lastDot,file.size(),std::string(""));
        listOut.push_back(file);
    }
    clearDirectoryContent(nb,list);
    return true;
}

// EOF
