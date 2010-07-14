#ifndef ADM_WINDOW_INFO_H
#define ADM_WINDOW_INFO_H
typedef struct
{
    void *display;
    int  window;
    int x;
    int y;
    int width;
    int height;
} GUI_WindowInfo;
#endif
