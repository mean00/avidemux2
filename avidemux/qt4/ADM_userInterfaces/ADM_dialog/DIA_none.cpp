
/***************************************************************************
                          DIA_none
    copyright            : (C) 2006 by mean
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
#include "ADM_default.h"
//#include "ADM_videoFilter.h"

#include "ADM_encoderConf.h"
//#include "ADM_audioEncoder/include/audioencoder_enum.h"


#ifdef USE_XX_XVID 
#include "xvid.h"
int  DIA_getXvidCompressParams(COMPRESSION_MODE * mode, uint32_t * qz,
		      uint32_t * br,uint32_t *fsize,xvidEncParam *param){return 0;}
#endif



uint8_t DIA_videoCodec(int *codecIndex) {return 0;}
uint8_t DIA_audioCodec( int *codec ) {return 0;}
uint8_t DIA_quota(char *) {return 0;}
uint8_t DIA_RecentFiles( char **name ){return 0;}
uint8_t DIA_glyphEdit(void){ return 0;};
int32_t UI_readJog(void) {return 0;}
struct THRESHOLD_PARAM;
struct ADMVideoThreshold;
/*uint8_t DIA_threshold(AVDMGenericVideoStream *in,
                      ADMVideoThreshold * thresholdp,
                      THRESHOLD_PARAM * param) { return 255; }

struct SWISSARMYKNIFE_PARAM;
struct ADMVideoSwissArmyKnife;
struct MenuMapping;
uint8_t DIA_SwissArmyKnife (AVDMGenericVideoStream * in,
                            ADMVideoSwissArmyKnife * sakp,
                            SWISSARMYKNIFE_PARAM * param,
                            const MenuMapping * menu_mapping,
                            uint32_t menu_mapping_count) { return 255; }
struct ERASER_PARAM;
struct ADMVideoEraser;
uint8_t DIA_eraser (AVDMGenericVideoStream * in,
                    ADMVideoEraser * eraserp,
                    ERASER_PARAM * param,
                    const MenuMapping * menu_mapping,
                    uint32_t menu_mapping_count) { return 255; }
struct PARTICLE_PARAM;
struct ADMVideoParticle;
uint8_t DIA_particle (AVDMGenericVideoStream *in,
                      ADMVideoParticle * particlep,
                      PARTICLE_PARAM * param,
                      const MenuMapping * menu_mapping,
                      uint32_t menu_mapping_count) { return 255; }
*/

//EOF
