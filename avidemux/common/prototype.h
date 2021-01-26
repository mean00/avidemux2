// GUI
#define ADM_SCALE_SIZE 10000
bool GUI_GoToKFrameTime(uint64_t exactTime);
bool GUI_GoToFrame(uint32_t frame);

#if 0
// RGB
void    GUI_RGBDisplay(uint8_t *dis,uint32_t w,uint32_t h,void *widg);

// Render
uint8_t	 GUI_Refresh( void );
uint8_t	GUI_XvRedraw(void);

// not in callback.h to avoid importing COMPRESSION MODE in interface.cpp

 //
 //
 //
void editorReignitPreview( void );
void editorKillPreview( void );
void editorUpdatePreview(uint32_t framenum)    ;

/**
	If set to 1, means video is in process mode_preview
	If set to 0, copy mode
*/
EXTERN uint32_t audioProcessMode(int dex);
/**
	If set to 1, means video is in process mode_preview
	If set to 0, copy mode
*/
EXTERN uint32_t videoProcessMode(void);

#endif

// EOF

