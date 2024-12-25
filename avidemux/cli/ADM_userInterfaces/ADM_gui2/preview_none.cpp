#include "config.h"
// clang format off
#include "ADM_assert.h"
#include "ADM_cpp.h"
#include "ADM_edit.hxx"
#include "GUI_render.h"
#include "fourcc.h"
// clang format on

void UI_rgbDraw(void *widg, uint32_t w, uint32_t h, uint8_t *ptr)
{
}
void UI_updateDrawWindowSize(void *win, uint32_t w, uint32_t h)
{
}
void UI_getWindowInfo(void *draw, GUI_WindowInfo *xinfo)
{
}
void *UI_getDrawWidget(void)
{
    return NULL;
}

void DIA_previewInit(uint32_t width, uint32_t height)
{
}
uint8_t DIA_previewUpdate(uint8_t *buffer)
{
    return 1;
}
void DIA_previewEnd(void)
{
}
uint8_t DIA_previewStillAlive(void)
{
    return 1;
}
