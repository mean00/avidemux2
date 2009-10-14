
// RGB
void    GUI_RGBDisplay(uint8_t *dis,uint32_t w,uint32_t h,void *widg);


// Render

uint8_t	 GUI_Refresh( void );
uint8_t	GUI_XvRedraw(void);

// GUI
#define ADM_SCALE_SIZE 10000
void GUI_GoToKFrameTime(uint64_t frame); // same as below execpt								                     // closest previous frame
int GUI_GoToFrame(uint32_t frame);
// not in callback.h to avoid importing COMPRESSION MODE in interface.cpp


 //
 //
 //
void editorReignitPreview( void );
void editorKillPreview( void );
void editorUpdatePreview(uint32_t framenum)    ;


// EOF

