#pragma once

// clang-format off
#include "ADM_default.h"
#include "config.h"
#include "GUI_render.h"
#include "DIA_coreToolkit.h"
#include "GUI_accelRender.h"
#include "GUI_nullRender.h"
#include "GUI_renderInternal.h"
#include "ADM_qtx.h"
#include "ADM_colorspace.h"
#include "DIA_uiTypes.h"
// clang-format on
//
//
//_______________________________________________________
#define TRY_RENDERER_INTERNAL(clss, create, name)                                                                      \
    VideoRenderBase *spawn = create clss();                                                                            \
    ADM_info("trying " name "\n");                                                                                     \
    r = spawn->init(&ctx.xinfo, ctx.phyW, ctx.phyH, ctx.lastZoom);                                                     \
    if (!r)                                                                                                            \
    {                                                                                                                  \
        delete spawn;                                                                                                  \
        spawn = NULL;                                                                                                  \
        ADM_warning(name " init failed\n");                                                                            \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        ADM_info(name " init ok\n");                                                                                   \
        return spawn;                                                                                                  \
    }
#define TRY_RENDERER_QT(spawn, name)                                                                                   \
    if (QT_X11_ENGINE == admDetectQtEngine())                                                                          \
    {                                                                                                                  \
        TRY_RENDERER_INTERNAL(spawn, , name)                                                                           \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        ADM_info("Disabling %s because of Wayland use\n", #name);                                                      \
    }
#define TRY_RENDERER_SPAWN_ALL(spawn, name)                                                                            \
    TRY_RENDERER_INTERNAL(spawn, , name)                                                                               \
//
