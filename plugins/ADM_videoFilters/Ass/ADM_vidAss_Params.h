#ifndef ADM_VID_ASS_PARAM_H
#define ADM_VID_ASS_PARAM_H

typedef struct 
{
        float         font_scale, line_spacing;
        uint32_t      top_margin, bottom_margin;
        ADM_filename  *subfile, *fonts_dir;
        uint32_t      extract_embedded_fonts;
} ASSParams;

#endif
