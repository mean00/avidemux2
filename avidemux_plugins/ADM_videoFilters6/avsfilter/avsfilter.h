/***************************************************************************
                          AVSfilter.cpp  -  description
                             -------------------
    begin                : 28-04-2008
    copyright            : (C) 2008 by fahr
    email                : fahr at inbox dot ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef AVSFILTER__
#define AVSFILTER__

typedef struct
{
  char *pipename;
  int hpipe;
  int flags;
} AVS_PIPES;

#define PIPE_LOADER_READ 0
#define PIPE_LOADER_WRITE 1
#define PIPE_FILTER_WRITE 2

#define CMD_PIPE_NUM (PIPE_FILTER_WRITE+1)

/*typedef struct
{
 ADM_filename *wine_app;
 ADM_filename *avs_script;
 ADM_filename *avs_loader;
 uint32_t script_mtime, script_ctime; // script timestamp
 uint32_t pipe_timeout;
} AVS_PARAM;*/

typedef struct
{
  avsfilter_config param;
  AVS_PIPES avs_pipes[CMD_PIPE_NUM];
  int order;
  FilterInfo input_info, output_info;
  int RefCounter;
  void *next_wine_loader;
} WINE_LOADER;

typedef struct
{
  AVS_PIPES *avs_pipes;
  FILE *pfile;
} TPARSER;

class avsfilter:public ADM_coreVideoFilter
{
protected:
    ADM_coreVideoFilter *in;
    ADMImage *_uncompressed;
    unsigned char *tmp_buf;
    char *prefs_name; // pointer to filename with preferences
    //  VideoCache *vidCache;
    uint32_t in_frame_sz, out_frame_sz;
    avsfilter_config param;
  virtual bool SetParameters(avsfilter_config *newparam);
  int order;
  WINE_LOADER *wine_loader;
//  AVS_PARAM *_param;
/*  bool wine_load, avs_load;*/
public:
  avsfilter( ADM_coreVideoFilter *in,CONFcouple *setup);
  virtual ~avsfilter();
  virtual bool getNextFrame(uint32_t *fn, ADMImage *image);
  virtual bool configure(void);
  virtual bool getCoupledConf( CONFcouple **couples);
  virtual void setCoupledConf( CONFcouple *couples);
  virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
};

/**
 * Class to cleanup all data after filter unload completely
 */
class AVSTerminate
{
public:
  AVSTerminate() {printf("Terminate class is calling in start\n");}
  virtual ~AVSTerminate();
};

#endif
