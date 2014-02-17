/***************************************************************************
 avsfilter.cpp  -  description
 -------------------
 begin                : 28-04-2008
 copyright            : (C) 2008-2013 by fahr
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

#ifndef AVS_WINE_BINARY_PATH
#error "AVS_WINE_BINARY_PATH not set!!!"
#endif

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_files.h"
#include "DIA_factory.h"
#include "errno.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#include "strnew.h"
#include "avspipecomm.h"
#include "avsfilterparam.h"
#include "avsfilterparam_desc.cpp"
#include "avsfilter.h"
#include "cdebug.h"

#define AVSFILTER_VERSION_INFO "AvsFilter, ver 0.12"
#define NOCOMPILE26

bool use_adv_protocol_avsfilter_to_pipesource = false;
bool use_adv_protocol_avsloader_to_avsfilter = false;

static WINE_LOADER *first_loader = NULL;
static AVSTerminate term;

#define MAXPATHLEN 512

/**
 * several functions for realize link of objects
 */
WINE_LOADER *find_object(int order,
                         char *avs_loader, char *avs_script,
                         time_t script_ctime, time_t script_mtime,
                         FilterInfo *input_info,
                         bool *full)
{
  WINE_LOADER *res = first_loader;

  while (res != NULL)
  {
    // check order first
    if (res->order == order)
    {
      // after we check other params
      if (!strcmp((char*)res->param.avs_loader, avs_loader) &&
          (!avs_script ||
           !strcmp((char*)res->param.avs_script, avs_script)) &&
          res->input_info.width == input_info->width &&
          res->input_info.height == input_info->height &&
          res->param.script_ctime == script_ctime &&
          res->param.script_mtime == script_mtime &&
          res->input_info.frameIncrement == input_info->frameIncrement &&
          res->input_info.totalDuration == input_info->totalDuration)
      {
/*        DEBUG_PRINTF("find_object : find %s %s\n",
                     (char*)res->_param.avs_loader,
                     (char*)res->_param.avs_script);*/
        if (full) *full = true;
      }
      else
      {
/*        DEBUG_PRINTF("find_object fail: %s %s %dx%d [%d - %d] ftime %X:%X != %s %s %dx%d [%d - %d] ftime %lX:%lX\n",
                     (char*)res->_param.avs_loader,
                     (char*)res->_param.avs_script,
                     res->input_info.width,
                     res->input_info.height,
                     res->input_info.orgFrame, res->input_info.orgFrame + res->input_info.nb_frames,
                     res->_param.script_ctime, res->_param.script_mtime,
                     avs_loader, avs_script, input_info->width, input_info->height,
                     input_info->orgFrame, input_info->orgFrame + input_info->nb_frames,
                     script_ctime, script_mtime);*/
        if (full) *full = false;
      }
      break;
    }
    res = (WINE_LOADER *)res->next_wine_loader;
  }
  return res;
}

void print_objects(void)
{
  WINE_LOADER *res = first_loader;

  while (res != NULL)
  {
/*    DEBUG_PRINTF("print_objects : %s %s %dx%d [%d - %d]\n",
                 (char*)res->_param.avs_loader,
                 (char*)res->_param.avs_script,
                 res->input_info.width,
                 res->input_info.height,
                 res->input_info.orgFrame, res->input_info.orgFrame + res->input_info.nb_frames);*/
    res = (WINE_LOADER *)res->next_wine_loader;
  }
  return;
}

void delete_object(WINE_LOADER *obj)
{
  WINE_LOADER *res = first_loader;

  if (res == obj)
  {
    first_loader = (WINE_LOADER *) obj->next_wine_loader;
    return;
  }

  while (res != NULL)
  {
    if (res->next_wine_loader == obj)
    {
      res->next_wine_loader = obj->next_wine_loader;
      break;
    }
    res = (WINE_LOADER *)res->next_wine_loader;
  }
}

void add_object(WINE_LOADER *obj)
{
  WINE_LOADER *res = first_loader;
  DEBUG_PRINTF("avsfilter : add_object start, res = %X\n", res);
  if (!res)
  {
    first_loader = obj;
    obj->next_wine_loader = NULL;
    return;
  }

  while (res != NULL)
  {
    if (res->next_wine_loader == NULL)
    {
      res->next_wine_loader = obj;
      obj->next_wine_loader = NULL;
      break;
    }
    res = (WINE_LOADER *) res->next_wine_loader;
  }
  DEBUG_PRINTF("avsfilter : add_object end\n");
}

//********************************************

void deinit_pipe(AVS_PIPES *avsp)
{
  if (avsp->hpipe != -1)
  {
    close(avsp->hpipe);
    avsp->hpipe = -1;
  }

  // if both link to pipename and pipename string exist
  DEBUG_PRINTF("avsfilter : deinit_pipe %X\n", avsp->pipename);
  DEBUG_PRINTF("avsfilter : deinit_pipe %s\n", avsp->pipename);
  remove(avsp->pipename);
  if (avsp->pipename && *avsp->pipename)
  {
    void *ptr = avsp->pipename;
    avsp->pipename = NULL;
    ADM_dealloc (ptr);
  }
}

void deinit_pipes(AVS_PIPES *avsp, int num)
{
  int i;
  for (i = 0; i < num; i++) deinit_pipe(&avsp[i]);
}

#define MAX_PATH 1024

bool init_pipes (AVS_PIPES *avsp, int num, FILE *pfile)
{
  int i;
  for (i = 0; i < num; i++)
  {
    char sname[MAX_PATH];

    if (fscanf(pfile, "%s\n", sname) != 1) DEBUG_PRINTF_RED("fscanf error\n");
    else if (!(avsp[i].pipename = strnew(sname))) DEBUG_PRINTF_RED("strnew error\n");
    else if (remove(avsp[i].pipename)) DEBUG_PRINTF_RED("error remove file\n");
    else if (mkfifo(avsp[i].pipename, 0600))
     DEBUG_PRINTF_RED("mkfifo error create fifo file %s, errno %d\n",
	         avsp[i].pipename, errno);
    else continue;

    deinit_pipes(avsp, i);
    return false;
  }

  return true;
}

bool open_pipes(AVS_PIPES *avsp, int num)
{
  int i;
  for (i = 0; i < num; i++)
  {
    DEBUG_PRINTF("avsfilter : try to open %s fifo\n", avsp[i].pipename);
    if ((avsp[i].hpipe = open(avsp[i].pipename, avsp[i].flags)) == -1)
    {
      DEBUG_PRINTF_RED("avsfilter : failed open errno %d\n", errno);
      deinit_pipe(&avsp[i]);
      deinit_pipes(avsp, i);
      return false;
    }
  }

  DEBUG_PRINTF("all pipes open ok\n");

  return true;
}

bool pipe_test_filter(int hr, int hw)
{

  uint32_t test_send = (uint32_t) time(NULL);
  uint32_t test_r1 = 0;

  int sz1;

  DEBUG_PRINTF("avsfilter : pipe_test_filter prewrite\n");

  sz1 = write(hw, &test_send, sizeof(uint32_t));

  if (sz1 != sizeof(uint32_t)) return false;

  DEBUG_PRINTF("avsfilter : pipe_test_filter preread\n");

  sz1 = read(hr, &test_r1, sizeof(uint32_t));

  if (sz1 != sizeof(uint32_t) || (test_r1 != test_send)) return false;

  return true;
}

bool open_pipes_ok, wine_loader_down = false;

AVSTerminate::~AVSTerminate()
{
  WINE_LOADER *cur_loader = first_loader;
  int i = 0;
  DEBUG_PRINTF("Call terminate!!!\n");

  if (cur_loader)
    do {
      DEBUG_PRINTF("Count %d\n", i++);

      if (cur_loader->avs_pipes[PIPE_LOADER_WRITE].hpipe != -1)
      {
        send_cmd(cur_loader->avs_pipes[PIPE_LOADER_WRITE].hpipe,
                 UNLOAD_AVS_SCRIPT, NULL, 0);
        DEBUG_PRINTF("UNLOAD_AVS_SCRIPT try\n");
      }

      if (cur_loader->avs_pipes[PIPE_LOADER_WRITE].hpipe != -1)
      {
        send_cmd(cur_loader->avs_pipes[PIPE_LOADER_WRITE].hpipe,
                 UNLOAD_AVS_LOADER, NULL, 0);
        DEBUG_PRINTF("UNLOAD_AVS_LOADER try\n");
      }

      deinit_pipes(cur_loader->avs_pipes, CMD_PIPE_NUM);
    } while((cur_loader = (WINE_LOADER*)cur_loader->next_wine_loader) != NULL);
}

void *parse_wine_stdout(void *arg)
{
  char sname[MAX_PATH];
  TPARSER *tp = (TPARSER *)arg;
  FILE *pfile = tp->pfile;
  AVS_PIPES copy_pipes [CMD_PIPE_NUM];
  int i;
  for (i = 0; i < CMD_PIPE_NUM; i++)
  {
    memcpy (&copy_pipes[i], &tp->avs_pipes[i], sizeof(AVS_PIPES));
    if ((copy_pipes[i].flags & O_ACCMODE) == O_RDONLY)
      copy_pipes[i].flags = (copy_pipes[i].flags & ~O_ACCMODE) | O_WRONLY;
    else
      if ((copy_pipes[i].flags & O_ACCMODE) == O_WRONLY)
        copy_pipes[i].flags = (copy_pipes[i].flags & ~O_ACCMODE) | O_RDONLY;

    DEBUG_PRINTF("avsfilter : new.flags %X, old.flags %X\n",
           copy_pipes[i].flags, tp->avs_pipes[i].flags);
  }

  wine_loader_down = false;

  if (pfile)
  {
    time_t t = time(NULL);
    DEBUG_PRINTF("avsfilter : pthread time %s\n",
           ctime(&t));
    DEBUG_PRINTF("pthread start ok\n");
    while(fgets(sname, MAX_PATH, pfile) != NULL)
#ifdef DEBUGMSG
      printf("%s", sname);
#else
    ;
#endif
    DEBUG_PRINTF("End parse\n");
    pclose(pfile);

    wine_loader_down = true;

    // if pipes not open completely, then simple open from thread for close
    if (!open_pipes_ok)
    {
      DEBUG_PRINTF("avsfilter : loader down, try to close waiting (for open) main thread\n");
      if (open_pipes((AVS_PIPES*)&copy_pipes, CMD_PIPE_NUM))
      {
        DEBUG_PRINTF("avsfilter : open ok, try to deinit\n");
        //deinit_pipes((AVS_PIPES*)&copy_pipes, CMD_PIPE_NUM);
        DEBUG_PRINTF("avsfilter : deinit done\n");
      }
    }

  }
}

bool wine_start(char *wine_app, char *avsloader, AVS_PIPES *avs_pipes, int pipe_timeout)
{
  char sname[MAX_PATH];
  struct stat st;
  sprintf(sname, "%s %s %d", wine_app, avsloader, pipe_timeout);

  FILE *pfile = popen(sname, "r");
  if (!pfile)
  {
   DEBUG_PRINTF_RED("avsfilter : popen failed, errno %d, failed start app is : [%s]\n", errno, sname);
   return false;
  }

  if (fscanf(pfile, "%s\n", sname) != 1 ||
      stat(sname, &st) ||
      !S_ISDIR(st.st_mode))
  {
    DEBUG_PRINTF_RED("avsfilter : tmpdirname [%s] failed, errno %d[stat %d isdir %d]\n", sname, errno, stat(sname, &st), S_ISDIR(st.st_mode));
    pclose(pfile);
    return false;
  }

  DEBUG_PRINTF("avsfilter : good tmpdirname %s\n", sname);

  if (!init_pipes(avs_pipes, CMD_PIPE_NUM, pfile))
  {
    DEBUG_PRINTF_RED("init_pipes failed\n");
    pclose(pfile);
    return false;
  }

  time_t t = time(NULL);
  DEBUG_PRINTF("avsfilter : precreate thread time %s\n",
         ctime(&t));
  pthread_t thread;
  TPARSER tp = { avs_pipes, pfile };

  open_pipes_ok = false;

  if (pthread_create(&thread, NULL, parse_wine_stdout, &tp))
  {
    DEBUG_PRINTF_RED("Cannot pthread started...Errno %d\n",errno);
    deinit_pipes(avs_pipes, CMD_PIPE_NUM);
    return false;
  }

  t = time(NULL);
  DEBUG_PRINTF("avsfilter : preopen time %s\n",
         ctime(&t));

  if (!open_pipes(avs_pipes, CMD_PIPE_NUM) || wine_loader_down)
  {
    open_pipes_ok = true;
    DEBUG_PRINTF_RED("open_pipes failed\n");
    deinit_pipes(avs_pipes, CMD_PIPE_NUM);
    return false;
  }

  open_pipes_ok = true;

  if (pipe_test_filter (avs_pipes[PIPE_LOADER_READ].hpipe,
                        avs_pipes[PIPE_FILTER_WRITE].hpipe))
  {
    DEBUG_PRINTF("avsfilter : test pipe to filter ok\n");

    if (pipe_test_filter (avs_pipes[PIPE_LOADER_READ].hpipe,
                          avs_pipes[PIPE_LOADER_WRITE].hpipe))
    {
      DEBUG_PRINTF("avsfilter : test pipe to loader ok\n");
    }
    else
     goto error_pipe_test;
  }
  else
  {
    error_pipe_test:
    DEBUG_PRINTF_RED("Error test read/write pipes\n");
    deinit_pipes(avs_pipes, CMD_PIPE_NUM);
    return false;
  }

  DEBUG_PRINTF("wine start is ok\n");
  return true;
}

bool avs_start(FilterInfo *info, FilterInfo *avisynth_info,
               char *fname, AVS_PIPES *avs_pipes, PITCH_DATA *pd_pipe_source, PITCH_DATA *pd_avsloader)
{
 DEBUG_PRINTF("avsfilter : avs_start()\n");
 DEBUG_PRINTF("avsfilter : %X %X %s %X\n",
              avs_pipes[PIPE_LOADER_WRITE].hpipe,
              avs_pipes[PIPE_FILTER_WRITE].hpipe,
              fname, info);
 DEBUG_PRINTF("avsfilter : avs_start info : frameIncrement %lu totalDuration %llu\n",
              info->frameIncrement, info->totalDuration);

 ADV_Info aii, aio;
 aii.width = info->width;
 aii.height = info->height;
 aii.nb_frames = info->totalDuration / info->frameIncrement;
 aii.encoding = MAGIC_ADV_PROTOCOL_VAL;
 aii.codec = 0;
 aii.fps1000 = ADM_Fps1000FromUs(info->frameIncrement);
 aii.orgFrame = 0;
 DEBUG_PRINTF("avsfilter : send ADV_Info to avsloader [fps1000 = %d, nb_frames = %d]\n", aii.fps1000, aii.nb_frames);
 if (!send_cmd(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                LOAD_AVS_SCRIPT, fname,
                strlen(fname) + sizeof("\0")) ||
      !send_cmd(avs_pipes[PIPE_FILTER_WRITE].hpipe,
                SET_CLIP_PARAMETER, &aii,
                sizeof(aii)))
  {
    DEBUG_PRINTF_RED("avsfilter : cannot set script name or set clip parameters\n");
    deinit_pipes(avs_pipes, CMD_PIPE_NUM);
    return false;
  }

  // get avisynth frame info
 PIPE_MSG_HEADER msg;
 if (!receive_cmd(avs_pipes[PIPE_LOADER_READ].hpipe, &msg))
 {
  DEBUG_PRINTF_RED("avsfilter : cannot receive command (SEND_PITCH_DATA_PIPE_SOURCE, OR SET_CLIP_PARAMETER)\n");
  deinit_pipes(avs_pipes, CMD_PIPE_NUM);
  return false;
 }

 switch (msg.avs_cmd)
 {
 case SEND_PITCH_DATA_PIPE_SOURCE:
     if (!receive_data(avs_pipes[PIPE_LOADER_READ].hpipe, &msg, pd_pipe_source))
     {
      DEBUG_PRINTF_RED("avsfilter : cannot receive SEND_PITCH_DATA_PIPE_SOURCE\n");
      deinit_pipes(avs_pipes, CMD_PIPE_NUM);
      return false;
     }
     DEBUG_PRINTF("avsfilter : receive SEND_PITCH_DATA_PIPE_SOURCE YUV = %d %d %d\n", pd_pipe_source->pitchY, pd_pipe_source->pitchU, pd_pipe_source->pitchV);
     if (!receive_cmd(avs_pipes[PIPE_LOADER_READ].hpipe, &msg) ||
         msg.avs_cmd != SET_CLIP_PARAMETER)
     {
      DEBUG_PRINTF_RED("avsfilter : cannot receive SET_CLIP_PARAMETER header message\n");
      deinit_pipes(avs_pipes, CMD_PIPE_NUM);
      return false;
     }
 case SET_CLIP_PARAMETER:
     if (!receive_data(avs_pipes[PIPE_LOADER_READ].hpipe, &msg, &aio))
     {
      DEBUG_PRINTF_RED("avsfilter : cannot receive avisynth clip parameters\n");
      deinit_pipes(avs_pipes, CMD_PIPE_NUM);
      return false;
     }
     break;
 default :
     DEBUG_PRINTF_RED("avsfilter : receive unknown command %d\n", msg.avs_cmd);
     deinit_pipes(avs_pipes, CMD_PIPE_NUM);
     return false;
     break;
 }

  DEBUG_PRINTF("avsfilter : receive ADV_Info from avsloader [fps1000 = %d, nb_frames = %d]\n", aio.fps1000, aio.nb_frames);
  avisynth_info->width = aio.width;
  avisynth_info->height = aio.height;
  avisynth_info->frameIncrement = ADM_UsecFromFps1000(aio.fps1000);
  avisynth_info->totalDuration = aio.nb_frames * avisynth_info->frameIncrement;
  if (aio.encoding == MAGIC_ADV_PROTOCOL_VAL)
  {
   DEBUG_PRINTF("avsfilter : send GET_PITCH_DATA to avsloader\n");
   if (!send_cmd(avs_pipes[PIPE_LOADER_WRITE].hpipe,
                 GET_PITCH_DATA, NULL, 0))
   {
    DEBUG_PRINTF_RED("avsfilter : cannot send GET_PITCH_DATA\n");
    deinit_pipes(avs_pipes, CMD_PIPE_NUM);
    return false;
   }
  }

  // correct avisynth_info for span of frames, calculate fps change metrics
/*  float k_fps;
  k_fps = float(avisynth_info->frameIncrement) / float(info->frameIncrement);
  DEBUG_PRINTF("avsfilter : FPS change metrics %f\n", k_fps);
  avisynth_info->nb_frames = int (info->nb_frames * k_fps);
  avisynth_info->orgFrame = int (info->orgFrame * k_fps);
  DEBUG_PRINTF("avsfilter : Calculate new span for avisynth script [%d - %d]\n",
               avisynth_info->orgFrame,avisynth_info->orgFrame + avisynth_info->nb_frames);*/
  return true;
}
#ifdef VERSION_2_5

DECLARE_VIDEO_FILTER(avsfilter,
                     0,0,12,
                     ADM_UI_ALL,
                     VF_MISC,
                     "avsfilter",
                     "avsfilter",
                     "Use avisynth script as video filter.");

#else
extern "C"
{
  SCRIPT_CREATE(FILTER_create_fromscript,avsfilter,avsParam);
  BUILD_CREATE(FILTER_create,avsfilter);

  char *FILTER_getName(void)
  {
    return AVSFILTER_VERSION_INFO;
  }

  char *FILTER_getDesc(void)
  {
    return "This filter do intermediate processing via avisynth script";
  }

  uint32_t FILTER_getVersion(void)
  {
    return 1;
  }
  uint32_t FILTER_getAPIVersion(void)
  {
    return ADM_FILTER_API_VERSION;
  }
}
#endif

const char *avsfilter::getConfiguration(void)
{
  static char buf[MAXPATHLEN];

  snprintf((char *)buf, MAXPATHLEN, "wine_app : %s\n loader : %s\n script : %s\npipe timeout %d\n",
           param.wine_app, param.avs_loader, param.avs_script, param.pipe_timeout);
  return buf;
}

bool  avsfilter_config_jserialize(const char *file, const avsfilter_config *key);
bool  avsfilter_config_jdeserialize(const char *file, const ADM_paramList *tmpl,avsfilter_config *key);

bool avsfilter::configure(void)
{
 DEBUG_PRINTF("avsfilter : before dialog init\n");
 print_objects();

#define PX(x) &(param.x)
 diaElemFile wine_app(0,(char**)PX(wine_app),
                      QT_TR_NOOP("_wine app file:"), NULL,
                      QT_TR_NOOP("Select wine filename[wine/cedega/etc.]"));
 diaElemFile loaderfile(0,(char**)PX(avs_loader),
                        QT_TR_NOOP("_loader file:"), NULL,
                        QT_TR_NOOP("Select loader filename[avsload.exe]"));
 diaElemFile avsfile(0,(char**)PX(avs_script),
                     QT_TR_NOOP("_avs file:"), NULL,
                     QT_TR_NOOP("Select avs filename[*.avs]"));
 diaElemUInteger pipe_timeout(PX(pipe_timeout),QT_TR_NOOP("_pipe timeout:"),1,30);

 diaElem *elems[4]={&wine_app, &loaderfile, &avsfile, &pipe_timeout};

 if( diaFactoryRun(QT_TR_NOOP("AvsFilter config"), 4, elems))
 {
  bool res = false;

  DEBUG_PRINTF("avsfilter : configure before SetParameters\n");

  // if script/loader names are exist, then taste config
  if (param.avs_loader && strlen((const char*)param.avs_loader) &&
      param.avs_script && strlen((const char*)param.avs_script) &&
      param.wine_app && strlen((const char*)param.wine_app))
  {
   struct stat st;
   if (stat((char*)param.avs_script, &st) != 0)
   {
    DEBUG_PRINTF_RED("avsfilter : cannot stat script file\n");
    return 0;
   }

   param.script_mtime = st.st_mtime; // store timestamp
   param.script_ctime = st.st_ctime;

   print_objects();
   res = SetParameters(&param);
   if (res)
    avsfilter_config_jserialize(prefs_name, &param);

   DEBUG_PRINTF("avsfilter : configure before save prefs [%s][%s]\n",
                param.avs_script, param.avs_loader);
   // if setparameters are ok and (therefore) avs_script and avs_loader exist
   // we store this parameters in filter preferences
   DEBUG_PRINTF("avsfilter : after save prefs info : frameIncrement %lu totalDuration %llu\n",
                info.frameIncrement, info.totalDuration);

   DEBUG_PRINTF("avsfilter : configure exit ok\n");
   return res;
  }
 }
 return 0;
}

//#define SET_AVS(i,x,y,z) {wine_loader->avs_pipes[i].pipename = x; wine_loader->avs_pipes[i].hpipe = y; wine_loader->avs_pipes[i].flags = z;}

bool avsfilter::SetParameters(avsfilter_config *newparam)
{
  bool full_exact = false;
  DEBUG_PRINTF("avsfilter : SetParameters\n");

  // find corresponding loader/script
  WINE_LOADER *loader = find_object(order,
                                    (char*)newparam->avs_loader,
                                    (char*)newparam->avs_script,
                                    newparam->script_ctime, newparam->script_mtime,
                                    &info,
                                    &full_exact);
  // if loader not found
  if (!loader)
  {
    DEBUG_PRINTF("avsfilter : SetParameters no loader found\n");
    loader = new (WINE_LOADER);
    loader->avs_pipes[0].flags = O_RDONLY;
    loader->avs_pipes[1].flags = O_WRONLY;
    loader->avs_pipes[2].flags = O_WRONLY;
    loader->RefCounter = 0;
    loader->param.avs_script = NULL;
    loader->param.avs_loader = NULL;

    if (!wine_start((char*)newparam->wine_app, (char*)newparam->avs_loader, loader->avs_pipes, newparam->pipe_timeout))
    {
      DEBUG_PRINTF_RED("avsfilter : wine_start unsuccessful start!\n");
      delete loader;
      deref_wine_loader:
      if (wine_loader)
      {
        wine_loader->RefCounter--;
        wine_loader = NULL;
      }
      return false;
    }

    DEBUG_PRINTF("avsfilter : SetParameters success start wine\n");
    loader->order = order;
    add_object(loader);
  }

  // all parameters are NOT matched [order only]
  if (!full_exact)
  {
    DEBUG_PRINTF("avsfilter : SetParameters !full_exact\n");

    // matched only order (need reload with new script/geometry/etc)
    if (!avs_start(&info, &loader->output_info, (char*)newparam->avs_script, loader->avs_pipes, &pd_pipe_source, &pd_avsloader))
    {
      DEBUG_PRINTF_RED("avsfilter : SetParameters fail avs_start\n");
      delete_object(loader);
      goto deref_wine_loader;
    }

    DEBUG_PRINTF("avsfilter : SetParameters avs_start ok\n");
    loader->RefCounter = 0;
    memcpy(&loader->input_info, &info, sizeof(info));
    loader->param.avs_loader = ADM_strdup ((char*)newparam->avs_loader);
    loader->param.avs_script = ADM_strdup ((char*)newparam->avs_script);
    loader->param.script_ctime = newparam->script_ctime; // store timestamp
    loader->param.script_mtime = newparam->script_mtime;
  }

  if (wine_loader && wine_loader != loader) wine_loader->RefCounter--;
  wine_loader = loader;
  wine_loader->RefCounter++;
  out_frame_sz = ((loader->output_info.width * loader->output_info.height) * 3) >>1;
  // 22.11 fix size of output filter with fullexact found loader
  info.width = loader->output_info.width;
  info.height = loader->output_info.height;
  info.frameIncrement = loader->output_info.frameIncrement;
  info.totalDuration = loader->output_info.totalDuration;
  /*  info.fps1000 = loader->output_info.fps1000;
  info.nb_frames = loader->output_info.nb_frames;
  info.orgFrame = loader->output_info.orgFrame;*/

/*   DEBUG_PRINTF("avsfilter : clip info : geom %d:%d fps1000 %d num_frames %d\n",
 info.width, info.height, info.fps1000, info.nb_frames);*/
  DEBUG_PRINTF("avsfilter : clip info : geom %d:%d frameIncrement %lu totalDuration %llu\n",
               info.width, info.height, info.frameIncrement, info.totalDuration);

  DEBUG_PRINTF("avsfilter : SetParameters return Ok\n");
  return true;
}

avsfilter::avsfilter(ADM_coreVideoFilter *in,
                     CONFcouple *couples) : ADM_coreVideoFilter(in,couples)
{
  ADM_assert(in);
  tmp_buf = NULL;
  in=in;
  DEBUG_PRINTF("Create AVSfilter(%X), AVDMGenericVideoStream %X\n", this, in);

  wine_loader = NULL;
//  param = new (avsfilter_config);
  DEBUG_PRINTF("avsfilter : preconstructor info : frameIncrement %lu totalDuration %llu\n",
               info.frameIncrement, info.totalDuration);
  memcpy(&info, previousFilter->getInfo(),sizeof(info));
  DEBUG_PRINTF("avsfilter : constructor info : frameIncrement %lu totalDuration %llu\n",
               info.frameIncrement, info.totalDuration);

//  info.encoding=1;
  //vidCache = NULL;

#define AVSFILTER_CONFIG_NAME "/avsfilter.config2"
  int sz_prefs_name = strlen (ADM_getUserPluginSettingsDir()) + sizeof(AVSFILTER_CONFIG_NAME) + 1;
  prefs_name = new char [sz_prefs_name];
  snprintf(prefs_name, sz_prefs_name, "%s%s", ADM_getUserPluginSettingsDir(), AVSFILTER_CONFIG_NAME);

  if (!couples || !ADM_paramLoad(couples,
                                 avsfilter_config_param,
                                 &param))
  {
   if (!avsfilter_config_jdeserialize(prefs_name, avsfilter_config_param, &param))
   {
    param.wine_app = ADM_strdup("wine");
    param.avs_script = ADM_strdup("test.avs");;
    param.avs_loader = ADM_strdup(AVS_WINE_BINARY_PATH"/avsload.exe");
    param.pipe_timeout = 10;
    param.script_ctime = 0;
    param.script_mtime = 0;
   }
  }

  DEBUG_PRINTF("avsfilter : constructor info #2: frameIncrement %lu totalDuration %llu\n",
               info.frameIncrement, info.totalDuration);

  DEBUG_PRINTF("avsfilter : wine_app %s avsloader %s avsscript %s\n",
               param.wine_app, param.avs_loader, param.avs_script);

  if (!SetParameters(&param))
  {
   DEBUG_PRINTF_RED("avsfilter : SetParameters return false\n");
   DEBUG_PRINTF("avsfilter : info after error: frameIncrement %lu totalDuration %llu\n",
                info.frameIncrement, info.totalDuration);
   return;
  }

  DEBUG_PRINTF("avsfilter : constructor info #3: frameIncrement %lu totalDuration %llu\n",
               info.frameIncrement, info.totalDuration);

#ifndef NOCOMPILE26
    if (prefs->get(FILTERS_AVSFILTER_WINE_APP, &tmp_str) == RC_OK &&
        strlen(tmp_str) > 0)
    {
      _param->wine_app = (ADM_filename*)ADM_strdup (tmp_str);
      DEBUG_PRINTF("avsfilter : wine_app from config is %s\n", _param->wine_app);
      ADM_dealloc(tmp_str);
    }

    if (prefs->get(FILTERS_AVSFILTER_AVS_SCRIPT, &tmp_str) == RC_OK &&
        strlen(tmp_str) > 0)
    {
      _param->avs_script = (ADM_filename*)ADM_strdup (tmp_str);
      DEBUG_PRINTF("avsfilter : avsscript from config is %s\n", _param->avs_script);
      ADM_dealloc(tmp_str);
    }

    if (prefs->get(FILTERS_AVSFILTER_AVS_LOADER, &tmp_str) == RC_OK &&
        strlen(tmp_str) > 0)
    {
      _param->avs_loader = (ADM_filename*)ADM_strdup (tmp_str);
      DEBUG_PRINTF("avsfilter : avsloader from config is %s\n", _param->avs_loader);
      ADM_dealloc(tmp_str);
    }
    prefs->get(FILTERS_AVSFILTER_PIPE_TIMEOUT, &_param->pipe_timeout);

    struct stat st;
    if (_param->avs_script)
      if (stat((char*)_param->avs_script, &st) != 0)
      {
        DEBUG_PRINTF_RED("avsfilter : cannot stat script file\n");
        return;
      }
      else
      {
        _param->script_mtime = st.st_mtime; // store timestamp
        _param->script_ctime = st.st_ctime;
      }
  }
#endif
  _uncompressed=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
  ADM_assert(_uncompressed);
  in_frame_sz = ((_uncompressed->_width * _uncompressed->_height) * 3) >>1;
  tmp_buf = (unsigned char*)ADM_alloc(PIPE_MAX_TRANSFER_SZ * 2);
  ADM_assert(tmp_buf);
  DEBUG_PRINTF("avsfilter : after constructor info : frameIncrement %lu totalDuration %llu\n",
               info.frameIncrement, info.totalDuration);

  if (_uncompressed->GetPitch(PLANAR_Y) == pd_pipe_source.pitchY &&
      _uncompressed->GetPitch(PLANAR_U) == pd_pipe_source.pitchU &&
      _uncompressed->GetPitch(PLANAR_V) == pd_pipe_source.pitchV)
  {
   use_adv_protocol_avsfilter_to_pipesource = true;
   DEBUG_PRINTF("avsfilter : use_adv_protocol_avsfilter_to_pipesource = true\n");
  }

//  vidCache=new VideoCache(16,in);
}

avsfilter::~avsfilter()
{
  if (wine_loader)
  {
    wine_loader->RefCounter--;
    if (!wine_loader->RefCounter) wine_loader = NULL;
  }

  if (tmp_buf) ADM_dezalloc(tmp_buf);

/*  if (vidCache)
  {
    delete vidCache;
    vidCache = NULL;
  }*/
}

void avsfilter::setCoupledConf( CONFcouple *couples)
{
 DEBUG_PRINTF("avsfilter : setCoupledConf\n");
 ADM_paramLoad(couples, avsfilter_config_param, &param);
}

bool avsfilter::getCoupledConf( CONFcouple **couples)
{
  //*couples=new CONFcouple(6);

//#define CSET(x)  (*couples)->setCouple((char *)#x,(_param.x))
 DEBUG_PRINTF("avsfilter : getCoupledConf\n");
 DEBUG_PRINTF("avsfilter : getCoupledConf info: frameIncrement %lu totalDuration %llu\n",
              info.frameIncrement, info.totalDuration);
 bool t =  ADM_paramSave(couples, avsfilter_config_param, &param);
 DEBUG_PRINTF("avsfilter : getCoupledConf info #2: frameIncrement %lu totalDuration %llu\n",
              info.frameIncrement, info.totalDuration);
 return t;

/*  CSET(wine_app);
  CSET(avs_script);
  CSET(avs_loader);
  CSET(pipe_timeout);
  CSET(script_mtime);
  CSET(script_ctime);*/
/*  DEBUG_PRINTF("avsfilter : getCoupledConf end\n");
  return 1;*/
}

bool receive_bit_blt(int h,
                     uint8_t* srcp, int src_pitch,
                     int row_size, int height)
{
 for (int y = height; y > 0; --y)
 {
  if (ppread(h, (void*)srcp, row_size) != row_size) return false;
  srcp += src_pitch;
 }
 return true;
}

bool send_bit_blt(int h,
                  uint8_t* srcp, int src_pitch,
                  int row_size, int height, unsigned char *data)
{
 unsigned char *org_data = data;
 for (int y = height; y > 0; --y)
 {
  int cp_sz;
  memcpy(data, (void*)srcp, row_size);
  srcp += src_pitch;
  data += row_size;
  cp_sz = data - org_data;
  if (cp_sz >= PIPE_MAX_TRANSFER_SZ || y == 1)
   if (ppwrite(h, (void*)org_data, cp_sz) != cp_sz) return false;
   else
   {
    data = org_data;
    DEBUG_PRINTF("avsfilter : send_bit_blt copy %d\n", cp_sz);
   }
 }
 return true;
}

bool avsfilter::getNextFrame(uint32_t *fn, ADMImage *data)
{
 uint32_t iframe = nextFrame;
 uint32_t frame, tmpframe;
 frame = iframe;

  DEBUG_PRINTF("avsfilter : receive getFrameNumberNoAlloc %d, wine_loader %X\n",
               frame, wine_loader);

  // check framenumber
  if (!wine_loader || ((iframe * wine_loader->output_info.frameIncrement) > wine_loader->output_info.totalDuration))
  {
   DEBUG_PRINTF("avsfilter : input framenumber (%d) is out of bounds [time %d > %d] \n", iframe,
                iframe * wine_loader->output_info.frameIncrement, wine_loader->output_info.totalDuration);
   return false;
  }

  FRAME_DATA fd = {frame};

  // send command to get filtered data
  if (!send_cmd(wine_loader->avs_pipes[PIPE_LOADER_WRITE].hpipe,
                use_adv_protocol_avsloader_to_avsfilter ? GET_FRAME_WITH_PITCH : GET_FRAME, (void*)&fd,
                sizeof(FRAME_DATA)))
  {
    DEBUG_PRINTF_RED("avsfilter : error send GET_FRAME to avsloader\n");
    return 0;
  }

  // read all data from avsloader and pipe dll
  PIPE_MSG_HEADER msg;
  while (receive_cmd(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe, &msg))
  {
    switch(msg.avs_cmd)
    {
    case SEND_PITCH_DATA_AVSLOADER:
        DEBUG_PRINTF("avsfilter : receive SEND_PITCH_DATA_AVSLOADER\n");
        if (!receive_data(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe, &msg, &pd_avsloader))
        {
         DEBUG_PRINTF_RED("avsfilter : cannot get SEND_PITCH_DATA_AVSLOADER\n");
         return 0;
        }
        DEBUG_PRINTF("avsfilter : receive SEND_PITCH_DATA_AVSLOADER YUV = %d %d %d\n", pd_avsloader.pitchY, pd_avsloader.pitchU, pd_avsloader.pitchV);
        if (data->GetPitch(PLANAR_Y) == pd_avsloader.pitchY &&
            data->GetPitch(PLANAR_U) == pd_avsloader.pitchU &&
            data->GetPitch(PLANAR_V) == pd_avsloader.pitchV)
        {
         use_adv_protocol_avsloader_to_avsfilter = true;
         DEBUG_PRINTF("avsfilter : use_adv_protocol_avsloader_to_avsfilter = true\n");
        }
        break;

      case GET_FRAME: // this request from pipe_source for input frame(s) to avisynth core
        DEBUG_PRINTF("avsfilter : receive GET_FRAME\n");
        if (!receive_data(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe,
                          &msg, &fd))
        {
          DEBUG_PRINTF_RED("\navsfilter : error receive data\n");
          return 0;
        }

        DEBUG_PRINTF("avsfilter : GET_FRAME number %d\n", fd.frame);
        tmpframe = fd.frame;
        DEBUG_PRINTF("avsfilter : %d but really get %d\n", fd.frame, tmpframe);
        //src=vidCache->getImage(tmpframe);
        if (!previousFilter->getNextFrame(&tmpframe, _uncompressed))
        {
         DEBUG_PRINTF("avsfilter : !!!OOPS!!!\n");
         return false;
        }

/*        DEBUG_PRINTF("avsfilter : in frame size %lu pitchYUV %d %d %d, widthYUV %d %d %d, heightYUV %d %d %d\n",
                     in_frame_sz, _uncompressed->GetPitch(PLANAR_Y), _uncompressed->GetPitch(PLANAR_U), _uncompressed->GetPitch(PLANAR_V),
                     _uncompressed->GetWidth(PLANAR_Y), _uncompressed->GetWidth(PLANAR_U), _uncompressed->GetWidth(PLANAR_V),
                     _uncompressed->GetHeight(PLANAR_Y), _uncompressed->GetHeight(PLANAR_U), _uncompressed->GetHeight(PLANAR_V));*/

        if (use_adv_protocol_avsfilter_to_pipesource)
        {
         uint32_t pitch_data_sizeY = _uncompressed->GetPitch(PLANAR_Y) * _uncompressed->GetHeight(PLANAR_Y);
         uint32_t pitch_data_sizeU = _uncompressed->GetPitch(PLANAR_U) * _uncompressed->GetHeight(PLANAR_U);
         uint32_t pitch_data_sizeV = _uncompressed->GetPitch(PLANAR_V) * _uncompressed->GetHeight(PLANAR_V);
         uint32_t pitch_data_size = pitch_data_sizeY + pitch_data_sizeU + pitch_data_sizeV;
         DEBUG_PRINTF("avsfilter : pitch frame size %lu\n", pitch_data_size);

         if (!send_cmd_with_specified_size(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe,
                                           PUT_FRAME_WITH_PITCH,
                                           (void*)&fd, sizeof(FRAME_DATA), pitch_data_size) ||
             ppwrite(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe, _uncompressed->GetReadPtr(PLANAR_Y), pitch_data_sizeY) != pitch_data_sizeY ||
             ppwrite(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe, _uncompressed->GetReadPtr(PLANAR_U), pitch_data_sizeU) != pitch_data_sizeU ||
             ppwrite(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe, _uncompressed->GetReadPtr(PLANAR_V), pitch_data_sizeV) != pitch_data_sizeV)
         {
          DEBUG_PRINTF_RED("avsfilter : error send uncompressed PITCH frame to dll\n");
          return 0;
         }
        }
        else
         // send frame to pipe_source
         if (!send_cmd_with_specified_size(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe,
                                           PUT_FRAME, (void*)&fd, sizeof(FRAME_DATA), in_frame_sz) ||
             !send_bit_blt(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe,
                           _uncompressed->GetReadPtr(PLANAR_Y), _uncompressed->GetPitch(PLANAR_Y),
                           _uncompressed->GetWidth(PLANAR_Y), _uncompressed->GetHeight(PLANAR_Y), tmp_buf) ||
             !send_bit_blt(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe,
                           _uncompressed->GetReadPtr(PLANAR_U), _uncompressed->GetPitch(PLANAR_U),
                           _uncompressed->GetWidth(PLANAR_U), _uncompressed->GetHeight(PLANAR_U), tmp_buf) ||
             !send_bit_blt(wine_loader->avs_pipes[PIPE_FILTER_WRITE].hpipe,
                           _uncompressed->GetReadPtr(PLANAR_V), _uncompressed->GetPitch(PLANAR_V),
                           _uncompressed->GetWidth(PLANAR_V), _uncompressed->GetHeight(PLANAR_V), tmp_buf))
         {
          DEBUG_PRINTF_RED("avsfilter : error send uncompressed frame to dll\n");
          return 0;
         }

        //YPLANE(_uncompressed),
        DEBUG_PRINTF("avsfilter : send data ok for frame %d\n", fd.frame);
        break;

      case PUT_FRAME_WITH_PITCH:
      case PUT_FRAME: // this request from avsload.exe with filtering data after avisynth
        DEBUG_PRINTF("avsfilter : receive %s, msg.sz %d\n", msg.avs_cmd == PUT_FRAME_WITH_PITCH ? "PUT_FRAME_WITH_PITCH" : "PUT_FRAME", msg.sz);
        if (msg.avs_cmd == PUT_FRAME && msg.sz != out_frame_sz + sizeof(FRAME_DATA))
        {
         DEBUG_PRINTF_RED("avsfilter : PUT_FRAME error : msg.sz [%d] != out_frame_sz+sizeof(FRAME_DATA) [%d,%d]\n",
                           msg.sz, out_frame_sz, sizeof(FRAME_DATA));
          return 0;
        }

//        DEBUG_PRINTF("avsfilter : read 1\n");
        if (!receive_data_by_size(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe,
                                  &fd, sizeof(FRAME_DATA)))
        {
          DEBUG_PRINTF_RED("avsfilter : receive data error#1\n");
          return 0;
        }

//        ADM_assert(fd.frame == (iframe + info.orgFrame));

/*        DEBUG_PRINTF("avsfilter : data->GetWidth(PLANAR_Y) %d data->GetHeight(PLANAR_Y) %d\n",
                     data->GetWidth(PLANAR_Y), data->GetHeight(PLANAR_Y));
        DEBUG_PRINTF("avsfilter : data->GetWidth(PLANAR_U) %d data->GetHeight(PLANAR_U) %d\n",
                     data->GetWidth(PLANAR_U), data->GetHeight(PLANAR_U));
        DEBUG_PRINTF("avsfilter : data->GetWidth(PLANAR_V) %d data->GetHeight(PLANAR_V) %d\n",
                     data->GetWidth(PLANAR_V), data->GetHeight(PLANAR_V));*/

        if (msg.avs_cmd == PUT_FRAME_WITH_PITCH)
        {
         uint32_t pitch_data_sizeY = data->GetPitch(PLANAR_Y) * data->GetHeight(PLANAR_Y);
         uint32_t pitch_data_sizeU = data->GetPitch(PLANAR_U) * data->GetHeight(PLANAR_U);
         uint32_t pitch_data_sizeV = data->GetPitch(PLANAR_V) * data->GetHeight(PLANAR_V);
         uint32_t pitch_data_size = pitch_data_sizeY + pitch_data_sizeU + pitch_data_sizeV;

         if (msg.sz != (pitch_data_size + sizeof(FRAME_DATA)))
         {
          DEBUG_PRINTF_RED("avsfilter : PUT_FRAME_WITH_PITCH error : msg.sz [%d] != pitch_data_size + sizeof(FRAME_DATA) [%d,%d]\n",
                           msg.sz, pitch_data_size, sizeof(FRAME_DATA));
          return 0;
        }

         if (ppread(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe, (void*)data->GetReadPtr(PLANAR_Y), pitch_data_sizeY) != pitch_data_sizeY ||
             ppread(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe, (void*)data->GetReadPtr(PLANAR_U), pitch_data_sizeU) != pitch_data_sizeU ||
             ppread(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe, (void*)data->GetReadPtr(PLANAR_V), pitch_data_sizeV) != pitch_data_sizeV)
         {
          DEBUG_PRINTF_RED("avsfilter : receive data error for PUT_FRAME_WITH_PITCH\n");
          return 0;
         }
        }
        else
        {
  //       DEBUG_PRINTF("avsfilter : read %d frame number Y plane\n", fd.frame);
         if (!receive_bit_blt(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe,
                              data->GetWritePtr(PLANAR_Y), data->GetPitch(PLANAR_Y),
                              data->GetWidth(PLANAR_Y), data->GetHeight(PLANAR_Y)))
         {
          DEBUG_PRINTF_RED("avsfilter : receive data error#2\n");
          return 0;
         }

//         DEBUG_PRINTF("avsfilter : read %d frame number U plane\n", fd.frame);
         if (!receive_bit_blt(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe,
                              data->GetWritePtr(PLANAR_U), data->GetPitch(PLANAR_U),
                              data->GetWidth(PLANAR_U), data->GetHeight(PLANAR_U)))
         {
          DEBUG_PRINTF_RED("avsfilter : receive data error#3\n");
          return 0;
         }
//         DEBUG_PRINTF("avsfilter : read %d frame number V plane\n", fd.frame);
         if (!receive_bit_blt(wine_loader->avs_pipes[PIPE_LOADER_READ].hpipe,
                              data->GetWritePtr(PLANAR_V), data->GetPitch(PLANAR_V),
                              data->GetWidth(PLANAR_V), data->GetHeight(PLANAR_V)))
         {
          DEBUG_PRINTF_RED("avsfilter : receive data error#4\n");
          return 0;
         }
        }
//        *len = out_frame_sz;
        DEBUG_PRINTF("avsfilter : copy data\n");
        DEBUG_PRINTF("avsfilter : data parameters %d:%d\n",
                     data->_width, data->_height);
        data->copyInfo(_uncompressed);
        data->Pts = _uncompressed->Pts;
        //        vidCache->unlockAll();
        *fn=nextFrame;
        nextFrame++;
        return true;
        break;
    }
  }
  return false;
}
