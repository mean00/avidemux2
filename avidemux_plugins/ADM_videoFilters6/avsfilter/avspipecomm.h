/***************************************************************************
 avspipecomm.h  -  description
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

#ifdef __MINGW32__
#define uint32_t DWORD
#endif

typedef enum
{
  LOAD_AVS_SCRIPT = 1,
  SET_CLIP_PARAMETER,
  GET_FRAME,
  PUT_FRAME, //4
  UNLOAD_AVS_SCRIPT,
  UNLOAD_AVS_LOADER,
  SEND_PITCH_DATA_PIPE_SOURCE,
  SEND_PITCH_DATA_AVSLOADER,
  GET_PITCH_DATA,
  GET_FRAME_WITH_PITCH,
  PUT_FRAME_WITH_PITCH
} AVS_CMD;

typedef struct
{
   uint32_t width;
   uint32_t height;
   uint32_t nb_frames;
   uint32_t encoding;
   uint32_t codec;
   uint32_t fps1000;
   uint32_t orgFrame;
} ADV_Info;

typedef struct
{
  AVS_CMD avs_cmd;
  int sz;
} PIPE_MSG_HEADER;

/*
 struct for GET_FRAME and PUT_FRAME command
 GET_FRAME - frame is set only
 PUT_FRAME - frame and frame_data[] set to corresponding value
 */
typedef struct 

{
  uint32_t frame;
  unsigned char frame_data[0];
} FRAME_DATA;

typedef struct
{
   uint32_t pitchY;
   uint32_t pitchU;
   uint32_t pitchV;
}PITCH_DATA;

bool send_cmd(int hw, AVS_CMD cmd,
              const void *data, int sz);

bool send_cmd_by_two_part(int hw, AVS_CMD cmd,
                          void *data1, int sz1,
                          void *data2, int sz2);

bool send_cmd_with_specified_size(int hw, AVS_CMD cmd,
                                  void *data, int sz1, int sz2);

bool receive_cmd(int hr, PIPE_MSG_HEADER *msg);

bool receive_data(int hr, PIPE_MSG_HEADER *msg,
                  void *data);

bool receive_data_by_size(int hr, void *data, int size);

int ppread(int h, void *data, int sz);
int ppwrite(int h, const void *data, int sz);

#define PIPE_MAX_TRANSFER_SZ 65536/2
#define MAGIC_ADV_PROTOCOL_VAL 0xADBACEED
