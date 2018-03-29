/*
 * Copyright (c) 2008-2009 Intel Corporation. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

static int scale_2dimage(unsigned char *src_img, int src_imgw, int src_imgh,
                         unsigned char *dst_img, int dst_imgw, int dst_imgh)
{
    int row=0, col=0;

    for (row=0; row<dst_imgh; row++) {
        for (col=0; col<dst_imgw; col++) {
            *(dst_img + row * dst_imgw + col) = *(src_img + (row * src_imgh/dst_imgh) * src_imgw + col * src_imgw/dst_imgw);
        }
    }

    return 0;
}



static int yuvgen_planar(int width, int height,
                         unsigned char *Y_start, int Y_pitch,
                         unsigned char *U_start, int U_pitch,
                         unsigned char *V_start, int V_pitch,
                         unsigned int fourcc, int box_width, int row_shift,
                         int field)
{
    int row, alpha;
    unsigned char uv_value = 0x80;

    /* copy Y plane */
    int y_factor = 1;
    if (fourcc == VA_FOURCC_YUY2) y_factor = 2;
    for (row=0;row<height;row++) {
        unsigned char *Y_row = Y_start + row * Y_pitch;
        int jj, xpos, ypos;

        ypos = (row / box_width) & 0x1;

        /* fill garbage data into the other field */
        if (((field == VA_TOP_FIELD) && (row &1))
            || ((field == VA_BOTTOM_FIELD) && ((row &1)==0))) { 
            memset(Y_row, 0xff, width);
            continue;
        }
        
        for (jj=0; jj<width; jj++) {
            xpos = ((row_shift + jj) / box_width) & 0x1;
            if (xpos == ypos)
                Y_row[jj*y_factor] = 0xeb;
            else 
                Y_row[jj*y_factor] = 0x10;

            if (fourcc == VA_FOURCC_YUY2) {
                Y_row[jj*y_factor+1] = uv_value; // it is for UV
            }
        }
    }
  
    /* copy UV data */
    for( row =0; row < height/2; row++) {

        /* fill garbage data into the other field */
        if (((field == VA_TOP_FIELD) && (row &1))
            || ((field == VA_BOTTOM_FIELD) && ((row &1)==0))) {
            uv_value = 0xff;
        }

        unsigned char *U_row = U_start + row * U_pitch;
        unsigned char *V_row = V_start + row * V_pitch;
        switch (fourcc) {
        case VA_FOURCC_NV12:
            memset(U_row, uv_value, width);
            break;
        case VA_FOURCC_YV12:
            memset (U_row,uv_value,width/2);
            memset (V_row,uv_value,width/2);
            break;
        case VA_FOURCC_YUY2:
            // see above. it is set with Y update.
            break;
        default:
            printf("unsupported fourcc in loadsurface.h\n");
            assert(0);
        }
    }

    
    return 0;
}



static int upload_surface(VADisplay va_dpy, VASurfaceID surface_id,
                          int box_width, int row_shift,
                          int field)
{
    VAImage surface_image;
    void *surface_p=NULL, *U_start = NULL,*V_start = NULL;
    VAStatus va_status;
    unsigned int pitches[3]={0,0,0};
    
    va_status = vaDeriveImage(va_dpy,surface_id,&surface_image);
    CHECK_VASTATUS(va_status,"vaDeriveImage");

    vaMapBuffer(va_dpy,surface_image.buf,&surface_p);
    assert(VA_STATUS_SUCCESS == va_status);

    pitches[0] = surface_image.pitches[0];
    switch (surface_image.format.fourcc) {
    case VA_FOURCC_NV12:
        U_start = (char *)surface_p + surface_image.offsets[1];
        V_start = (char *)U_start + 1;
        pitches[1] = surface_image.pitches[1];
        pitches[2] = surface_image.pitches[1];
        break;
    case VA_FOURCC_IYUV:
        U_start = (char *)surface_p + surface_image.offsets[1];
        V_start = (char *)surface_p + surface_image.offsets[2];
        pitches[1] = surface_image.pitches[1];
        pitches[2] = surface_image.pitches[2];
        break;
    case VA_FOURCC_YV12:
        U_start = (char *)surface_p + surface_image.offsets[2];
        V_start = (char *)surface_p + surface_image.offsets[1];
        pitches[1] = surface_image.pitches[2];
        pitches[2] = surface_image.pitches[1];
        break;
    case VA_FOURCC_YUY2:
        U_start = (char *)surface_p + 1;
        V_start = (char *)surface_p + 3;
        pitches[1] = surface_image.pitches[0];
        pitches[2] = surface_image.pitches[0];
        break;
    default:
        assert(0);
    }

    /* assume surface is planar format */
    yuvgen_planar(surface_image.width, surface_image.height,
                  (unsigned char *)surface_p, pitches[0],
                  (unsigned char *)U_start, pitches[1],
                  (unsigned char *)V_start, pitches[2],
                  surface_image.format.fourcc,
                  box_width, row_shift, field);
        
    vaUnmapBuffer(va_dpy,surface_image.buf);

    vaDestroyImage(va_dpy,surface_image.image_id);

    return 0;
}

/*
 * Upload YUV data from memory into a surface
 * if src_fourcc == NV12, assume the buffer pointed by src_U
 * is UV interleaved (src_V is ignored)
 */
static int upload_surface_yuv(VADisplay va_dpy, VASurfaceID surface_id,
                              int src_fourcc, int src_width, int src_height,
                              unsigned char *src_Y, unsigned char *src_U, unsigned char *src_V)
{
    VAImage surface_image;
    unsigned char *surface_p=NULL, *Y_start=NULL, *U_start=NULL;
    int Y_pitch=0, U_pitch=0, row;
    VAStatus va_status;
    
    va_status = vaDeriveImage(va_dpy,surface_id, &surface_image);
    CHECK_VASTATUS(va_status,"vaDeriveImage");

    vaMapBuffer(va_dpy,surface_image.buf,(void **)&surface_p);
    assert(VA_STATUS_SUCCESS == va_status);

    Y_start = surface_p;
    Y_pitch = surface_image.pitches[0];
    switch (surface_image.format.fourcc) {
    case VA_FOURCC_NV12:
        U_start = (unsigned char *)surface_p + surface_image.offsets[1];
        U_pitch = surface_image.pitches[1];
        break;
    case VA_FOURCC_IYUV:
        U_start = (unsigned char *)surface_p + surface_image.offsets[1];
        U_pitch = surface_image.pitches[1];
        break;
    case VA_FOURCC_YV12:
        U_start = (unsigned char *)surface_p + surface_image.offsets[2];
        U_pitch = surface_image.pitches[2];
        break;
    case VA_FOURCC_YUY2:
        U_start = surface_p + 1;
        U_pitch = surface_image.pitches[0];
        break;
    default:
        assert(0);
    }

    /* copy Y plane */
    for (row=0;row<src_height;row++) {
        unsigned char *Y_row = Y_start + row * Y_pitch;
        memcpy(Y_row, src_Y + row*src_width, src_width);
    }
  
    for (row =0; row < src_height/2; row++) {
        unsigned char *U_row = U_start + row * U_pitch;
        unsigned char *u_ptr = NULL, *v_ptr=NULL;
        int j;
        switch (surface_image.format.fourcc) {
        case VA_FOURCC_NV12:
            if (src_fourcc == VA_FOURCC_NV12) {
                memcpy(U_row, src_U + row * src_width, src_width);
                break;
            } else if (src_fourcc == VA_FOURCC_IYUV) {
                u_ptr = src_U + row * (src_width/2);
                v_ptr = src_V + row * (src_width/2);
            } else if (src_fourcc == VA_FOURCC_YV12) {
                v_ptr = src_U + row * (src_width/2);
                u_ptr = src_V + row * (src_width/2);
            }
            for(j = 0; j < src_width/2; j++) {
                U_row[2*j] = u_ptr[j];
                U_row[2*j+1] = v_ptr[j];
            }
            break;
        case VA_FOURCC_IYUV:
        case VA_FOURCC_YV12:
        case VA_FOURCC_YUY2:
        default:
            printf("unsupported fourcc in load_surface_yuv\n");
            assert(0);
        }
    }
    
    vaUnmapBuffer(va_dpy,surface_image.buf);

    vaDestroyImage(va_dpy,surface_image.image_id);

    return 0;
}
