/***************************************************************************
                          gui_navigate.cpp  -  description
                             -------------------

            GUI Part of get next frame, previous key frame, any frame etc...

    begin                : Fri Apr 12 2002
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

#include "config.h"

#include <math.h>

#include "avi_vars.h"
#include "ADM_userInterfaces/ADM_commonUI/GUI_ui.h"
#include "DIA_coreToolkit.h"
#include "ADM_assert.h"

#include "audio_out.h"
#include "ADM_audio/aviaudio.hxx"
#include "audioprocess.hxx"
#include "audioeng_buildfilters.h"
#include "gui_action.hxx"

#include "ADM_encoder/adm_encConfig.h"
#include "ADM_filter/vidVCD.h"
#include "ADM_encoder/ADM_vidEncode.hxx"

extern void setVideoEncoderSettings (COMPRESSION_MODE mode, uint32_t param,     uint32_t extraConf, uint8_t * extraData);

uint8_t A_autoDrive(Action action)
{
  uint32_t fq;
        //
        if(!currentaudiostream)
        {
          GUI_Error_HIG(QT_TR_NOOP("No audio track"),QT_TR_NOOP( "An audio track is necessary to create such file"));
                return 0;
        }

      
#define PSP_AUDIO_FQ 24000        
        switch(action)
        {
                case ACT_AUTO_IPOD:
					if(!setIPOD())
						return 0;

					// Video codec

					if(!videoCodecSelectByName("XVID4")) 
					{
						GUI_Error_HIG(QT_TR_NOOP("Codec Error"),QT_TR_NOOP( "Cannot select the MPEG-4 SP codec."));
						return 0;
					}
                    GUI_Info_HIG(ADM_LOG_INFO,"CODE DISABLED","XVID4 PROFILE FOR IPOD");
					//setIpod_Xvid4Preset();
					// Set mode & bitrate
					setVideoEncoderSettings(COMPRESS_CBR,400,0,NULL);
					// Audio Codec
					if((currentaudiostream->getInfo()->channels == 2)&& currentaudiostream->getInfo()->encoding == WAV_AAC)
						audio_setCopyCodec();
					else
					{
						if(audio_selectCodecByTag(WAV_AAC))
                        {
                            GUI_Error_HIG(QT_TR_NOOP("Codec Error"),QT_TR_NOOP( "No AAC audio encoder plugin found."));
                        }else
                        {
                            audioFilter_SetBitrate(128);

                            if (currentaudiostream->getInfo()->channels != 2)
                                setCurrentMixerFromString("STEREO");
                        }
					}

                      break;
//******************************** IPOD *******************************
                          
                          
                          break;
        		case ACT_AUTO_FLV:
        			// Check audio is mp3 @ 44.1/22.05/11.025 kHz MP3
        			if(currentaudiostream)
        			{
        				fq=currentaudiostream->getInfo()->frequency;
        				if(currentaudiostream->getInfo()->encoding==WAV_MP3 && (fq==44100 || fq==22050 || fq==11025))
        						{
        	                		audio_setCopyCodec();
        						}
        				else
        				{
                            if(audio_selectCodecByTag(WAV_MP3))
                            {
                                    audioFilter_SetBitrate(128);
                                    if(!audioSetOption("MP3DisableReservoir",1))
                                    {
        					 GUI_Error_HIG(QT_TR_NOOP("Codec Error"),
        					        QT_TR_NOOP( "The MP3 codec does not allow disabling reservoir.\nInstall lame plugin"));

                                    }
        					}else
        					{
        					 GUI_Error_HIG(QT_TR_NOOP("Codec Error"),
        					        QT_TR_NOOP( "You don't have LAME!.\nIt is needed to create FLV  video."));
                             }
        					 if(fq==44100 || fq==22050 || fq==11025)
        					         						{}
        					 else
        					 {
        						 audioFilterResample(22050);
        					 }
        					 
        				}
        			}
        			// Now video
        			 if(!videoCodecSelectByName("FLV1")) 
                    {
                      GUI_Error_HIG(QT_TR_NOOP("Codec Error"),QT_TR_NOOP( "Cannot select FLV1  codec."));
                        return 0;
                    }
        			break;
        			
                case ACT_AUTO_PSP:
                case ACT_AUTO_PSP_H264:
                    // Resize
                  if(action==ACT_AUTO_PSP_H264)
                  {
                    if(!setPSPFullRes()) return 0;
                  }
                  else
                  {
                    if(!setPSP()) return 0;
                  }
                    // Video codec
                    switch(action)
                    {
                    case ACT_AUTO_PSP:
                          {
#ifdef USE_XVID_4
                            if(!videoCodecSelectByName("XVID4")) 
#else
                            if(!videoCodecSelectByName("FFMpeg4"))            
#endif
                            {
                              GUI_Error_HIG(QT_TR_NOOP("Codec Error"),QT_TR_NOOP( "Cannot select mpeg4 sp codec."));
                                return 0;
                            }
                            // Set mode & bitrate 
                            setVideoEncoderSettings(COMPRESS_CBR,768,0,NULL);
                            fq=24000;
                          }
                          break;
                    case ACT_AUTO_PSP_H264:
                    {
						// FIXME
/*#ifdef USE_X264
                          videoCodecSelectByName("X264"); 
                          setPSP_X264Preset(); 
                          setVideoEncoderSettings(COMPRESS_CBR,768,0,NULL);
#endif */
                          fq=48000;
                    }
                    break;
                    
                    default:
                          ADM_assert(0);
                    }
                    // Audio Codec
                    if((currentaudiostream->getInfo()->frequency==fq)&&
                        (currentaudiostream->getInfo()->channels==2)&&
                        (currentaudiostream->getInfo()->encoding==WAV_AAC))
                    {
                        audio_setCopyCodec();
                    }
                    else
                    {


                          if(!audio_selectCodecByTag(WAV_AAC))
                            GUI_Error_HIG(QT_TR_NOOP("Codec Error"),
                                        QT_TR_NOOP( "You don't have FAAC!.\nIt is needed to create PSP compatible video."));
                                    // ? Needed ?
                          if(currentaudiostream->getInfo()->frequency!=fq)
                          {
                              audioFilterResample(fq);
                          }
                          audioFilter_SetBitrate(128);
                      }
                                break;
                case ACT_AUTO_VCD:
                                // Check video size
                                if(!setVCD()) return 0;
                                // Set codec
                                if(!videoCodecSelectByName("VCD"))  return 0;
                                // Set audio
                                // Check if it is ok
                               if((currentaudiostream->getInfo()->frequency==44100)&&
                                        (currentaudiostream->getInfo()->channels==2)&&
                                                (currentaudiostream->getInfo()->encoding==WAV_MP2)&&
                                                        (currentaudiostream->getInfo()->byterate==28000))
                                {
                                        audio_setCopyCodec();
                                }
                                else
                                {
                                        audio_selectCodecByTag(WAV_MP2);
                                        if(currentaudiostream->getInfo()->frequency!=44100)
                                        {
                                                audioFilterResample(44100);
                                        }
#warning use mixer!!
                                        audioFilter_SetBitrate(224);
                                        if(currentaudiostream->getInfo()->channels!=2)
                                        {
                                          setCurrentMixerFromString("DOLBY_PROLOGIC2");
                                        }else
                                        {
                                          setCurrentMixerFromString("NONE");
                                        }
                                }
                                break;
                case ACT_AUTO_SVCD:
                                // Check video size
                                if(!setSVCD()) return 0;
                                // Set codec
                                if(!videoCodecSelectByName("XSVCD"))  return 0;
                                setVideoEncoderSettings (COMPRESS_2PASS_BITRATE,2000,0,NULL);
                                if((currentaudiostream->getInfo()->frequency==44100)&&
                                        (currentaudiostream->getInfo()->channels==2)&&
                                                (currentaudiostream->getInfo()->encoding==WAV_MP2))
                                {
                                        audio_setCopyCodec();
                                }
                                else
                                {
                                        audio_selectCodecByTag(WAV_MP2);
                                        if(currentaudiostream->getInfo()->frequency!=44100)
                                        {
                                                audioFilterResample(44100);
                                        }
                                        if(currentaudiostream->getInfo()->channels!=2)
                                        {
                                          setCurrentMixerFromString("DOLBY_PROLOGIC2");
                                        }else
                                        {
                                          setCurrentMixerFromString("NONE");
                                        }
                                        audioFilter_SetBitrate(160);
                                }
                                break;
                                break;

                case ACT_AUTO_DVD:
                                // Check video size
                                if(!setDVD()) return 0;
                                // Set codec
                                if(!videoCodecSelectByName("XDVD"))  return 0;
                                // Set 2 pass, avg bitrate  6 M
                                setVideoEncoderSettings (COMPRESS_2PASS_BITRATE,6000,0,NULL);
                                // Set audio
                                if(currentaudiostream->getInfo()->frequency!=48000
                                        || (currentaudiostream->getInfo()->encoding!=WAV_MP2 &&
                                         currentaudiostream->getInfo()->encoding!=WAV_AC3))
                                {
                                        audio_selectCodecByTag(WAV_MP2);
                                        audioFilterResample(48000);
                                        audioFilter_SetBitrate(160);
                                        if(currentaudiostream->getInfo()->channels!=2)
                                        {
                                          setCurrentMixerFromString("DOLBY_PROLOGIC2");
                                        }else
                                        {
                                          setCurrentMixerFromString("NONE");
                                        }
                                }
                                else
                                {
                                        audio_setCopyCodec();
                                }
                                break;

        }
        // Set output format to mpeg PS
        // Except for PSP
        switch(action)
        {
          case ACT_AUTO_IPOD:
              UI_SetCurrentFormat(ADM_MP4);
              break;
          case ACT_AUTO_PSP:
          case ACT_AUTO_PSP_H264:
              UI_SetCurrentFormat(ADM_PSP);
              break;
          case ACT_AUTO_FLV:
        	  UI_SetCurrentFormat(ADM_FLV);
        	  break;
          default:
              UI_SetCurrentFormat(ADM_PS);
        }
        return 1;
}
//EOF
