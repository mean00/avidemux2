
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "ADM_default.h"
#include "audioencoder.h"
#include "ADM_render/GUI_renderInternal.h"
#include "IScriptEngine.h"

using std::vector;

extern int global_argc;
extern char **global_argv;
extern void initTranslator(void);
extern int automation(void);

int SliderIsShifted = 0;

//uint8_t DIA_getPartial(PARTIAL_CONFIG *param,AVDMGenericVideoStream *son,AVDMGenericVideoStream *previous) {return 0;}
uint8_t DIA_pipe(char **cmd,char **param) {return 0;}
//uint8_t DIA_vobsub(vobSubParam *param) {return 0;}
uint8_t DIA_quota(char *) {return 0;}
const char * GUI_getCustomJsScript(uint32_t nb) {return 0;}
const char * GUI_getCustomPyScript(uint32_t nb) {return 0;}
const char * GUI_getAutoPyScript(uint32_t nb) {return 0;}
uint8_t DIA_RecentFiles( char **name ) {return 0;}
uint8_t DIA_about( void ) {return 0;}
void DIA_Calculator(uint32_t *sizeInMeg, uint32_t *avgBitrate ) {}
void DIA_properties(void) {}
void DIA_log(void) {}
int GUI_handleVFilter (void) {return 0;}
int GUI_handleVPartialFilter (void) {return 0;}
uint8_t initGUI(const vector<IScriptEngine*>& scriptEngines) {return 1;}
void destroyGUI(void) {}
uint8_t DIA_job(uint32_t nb,char **name) {return 0;}
uint8_t DIA_resize(uint32_t *width,uint32_t *height,uint32_t *algo,uint32_t originalw,
                        uint32_t originalh,uint32_t fps) {return 0;}
uint8_t DIA_d3d(double *luma,double *chroma,double *temporal) {return 0;}
uint8_t DIA_kerneldeint(uint32_t *order, uint32_t *threshold, uint32_t *sharp,
                          uint32_t *twoway, uint32_t *map) {return 0;}
uint8_t DIA_4entries(char *title,uint32_t *left,uint32_t *right,uint32_t *top,uint32_t *bottom) {return 0;}
uint8_t DIA_videoCodec(int *codecIndex) {return 0;}
uint8_t DIA_audioCodec( int *codec ) {return 0;}
uint8_t DIA_dnr(uint32_t *llock,uint32_t *lthresh, uint32_t *clock,
			uint32_t *cthresh, uint32_t *scene) {return 0;}
uint8_t DIA_glyphEdit(void) {return 0;}
struct THRESHOLD_PARAM;
struct ADMVideoThreshold;
struct SWISSARMYKNIFE_PARAM;
struct ADMVideoSwissArmyKnife;
struct MenuMapping;
//uint8_t ADM_ocrUpdateNbLines(void *ui,uint32_t cur,uint32_t total) {return 0;}
//uint8_t ADM_ocrUpdateNbGlyphs(void *ui,uint32_t nbGlyphs) {return 0;}
//uint8_t ADM_ocrUpdateTextAndTime(void *ui,char *decodedString,char *timeCode) {	return 0;}
//uint8_t ADM_ocrDrawFull(void *d,uint8_t *data) {return 0;}
//uint8_t ADM_ocrUiEnd(void *d) {	return 0;}
//void *ADM_ocrUiSetup(void) {return 0;}
//uint8_t ADM_ocrSetRedrawSize(void *ui,uint32_t w,uint32_t h) {return 0;}
//ReplyType glyphToText(admGlyph *glyph,admGlyph *head,char *decodedString) {return ReplyOk;}
extern ADM_RENDER_TYPE UI_getPreferredRender(void);

static const UI_FUNCTIONS_T UI_Hooks=
    {
        ADM_RENDER_API_VERSION_NUMBER,
        UI_getWindowInfo,
        UI_updateDrawWindowSize,
        UI_rgbDraw,
        UI_getDrawWidget,
        UI_getPreferredRender

    };
int UI_Init(int nargc, char **nargv)
{
	initTranslator();

	global_argc = nargc;
	global_argv = nargv;
        ADM_renderLibInit(&UI_Hooks);
	return 0;
}

int UI_RunApp(void)
{
	if (global_argc >= 2)
		automation();

	printf("*********************************\n");
	printf("*********************************\n");
	printf("End of program..\n");
	printf("*********************************\n");
	printf("*********************************\n");

	return 0;
}

bool UI_End(void)
{
    return true;
}
void UI_closeGui(void) {};

bool UI_setDecoderName(const char *name) {return true;}
int32_t UI_readJog(void) {return 0;};
//EOF
