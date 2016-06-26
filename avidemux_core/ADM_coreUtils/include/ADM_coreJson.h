/**
        \file  ADM_coreJson.h
        \brief 
*/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_CORE_JSON_H
#define ADM_CORE_JSON_H

#include "ADM_coreUtils6_export.h"
#include "ADM_cpp.h"
#include "ADM_coreVideoEncoder.h"
#include "ADM_encoderConf.h"
#include "ADM_confCouple.h"

/**
    \class admJson
    \brief Wrap libjson writter
*/
class ADM_COREUTILS6_EXPORT admJson
{
protected:
       
         vector <void *>cookies;
         vector <string >nested;
         void *cookie;
        
public:
            admJson();
            ~admJson();
        bool addString(const char *key,const char *value);
        bool addString(const char *key,const std::string &value);
        bool addUint32(const char *key,const uint32_t value);
        bool addInt32(const char *key,const int32_t value);
        bool addFloat(const char *key,const float value);
	bool addDouble(const char *key,const double value);
        bool addBool(const char *key,const bool value);
        bool addCompressParam(const char *key, const COMPRES_PARAMS &param);

        bool addNode(const char *nodeName);
        bool endNode(void);

        

        bool dumpToFile(const char *file);

      
      
};
/**
    \class admJsonToCouple
    \brief create conf couple from a json file
*/
class ADM_COREUTILS6_EXPORT admJsonToCouple
{
protected:
        typedef struct  
            {
                string key;
                string value;
            }keyVal;
         vector <keyVal> readItems;
         bool scan(void *node,string name);

public:
        admJsonToCouple() {}
        ~admJsonToCouple() {}
        CONFcouple *readFromFile(const char *fileName);

};
#endif
