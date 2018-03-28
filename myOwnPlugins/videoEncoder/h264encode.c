/*
 * Copyright (c) 2007-2013 Intel Corporation. All Rights Reserved.
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
#include "sysdeps.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <va/va.h>
#include <va/va_enc_h264.h>
#include "va_display.h"

#define CHECK_VASTATUS(va_status,func)                                  \
    if (va_status != VA_STATUS_SUCCESS) {                               \
        fprintf(stderr,"%s:%s (%d) failed,exit\n", __func__, func, __LINE__); \
        exit(1);                                                        \
    }

#include "../loadsurface.h"

#define NAL_REF_IDC_NONE        0
#define NAL_REF_IDC_LOW         1
#define NAL_REF_IDC_MEDIUM      2
#define NAL_REF_IDC_HIGH        3

#define NAL_NON_IDR             1
#define NAL_IDR                 5
#define NAL_SPS                 7
#define NAL_PPS                 8
#define NAL_SEI			6

#define SLICE_TYPE_P            0
#define SLICE_TYPE_B            1
#define SLICE_TYPE_I            2
#define IS_P_SLICE(type) (SLICE_TYPE_P == (type))
#define IS_B_SLICE(type) (SLICE_TYPE_B == (type))
#define IS_I_SLICE(type) (SLICE_TYPE_I == (type))


#define ENTROPY_MODE_CAVLC      0
#define ENTROPY_MODE_CABAC      1

#define PROFILE_IDC_BASELINE    66
#define PROFILE_IDC_MAIN        77
#define PROFILE_IDC_HIGH        100
   
#define BITSTREAM_ALLOCATE_STEPPING     4096

#define SURFACE_NUM 16 /* 16 surfaces for source YUV */
#define SURFACE_NUM 16 /* 16 surfaces for reference */
static  VADisplay va_dpy;
static  VAProfile h264_profile = ~0;
static  VAConfigAttrib attrib[VAConfigAttribTypeMax];
static  VAConfigAttrib config_attrib[VAConfigAttribTypeMax];
static  int config_attrib_num = 0, enc_packed_header_idx;
static  VASurfaceID src_surface[SURFACE_NUM];
static  VABufferID  coded_buf[SURFACE_NUM];
static  VASurfaceID ref_surface[SURFACE_NUM];
static  VAConfigID config_id;
static  VAContextID context_id;
static  VAEncSequenceParameterBufferH264 seq_param;
static  VAEncPictureParameterBufferH264 pic_param;
static  VAEncSliceParameterBufferH264 slice_param;
static  VAPictureH264 CurrentCurrPic;
static  VAPictureH264 ReferenceFrames[16], RefPicList0_P[32], RefPicList0_B[32], RefPicList1_B[32];

static  unsigned int MaxFrameNum = (2<<16);
static  unsigned int MaxPicOrderCntLsb = (2<<8);
static  unsigned int Log2MaxFrameNum = 16;
static  unsigned int Log2MaxPicOrderCntLsb = 8;

static  unsigned int num_ref_frames = 2;
static  unsigned int numShortTerm = 0;
static  int constraint_set_flag = 0;
static  int h264_packedheader = 0; /* support pack header? */
static  int h264_maxref = (1<<16|1);
static  int h264_entropy_mode = 1; /* cabac */

static  char *coded_fn = NULL, *srcyuv_fn = NULL, *recyuv_fn = NULL;
static  FILE *coded_fp = NULL, *srcyuv_fp = NULL, *recyuv_fp = NULL;
static  unsigned long long srcyuv_frames = 0;
static  int srcyuv_fourcc = VA_FOURCC_NV12;
static  int calc_psnr = 0;

static  int frame_width = 176;
static  int frame_height = 144;
static  int frame_width_mbaligned;
static  int frame_height_mbaligned;
static  int frame_rate = 30;
static  unsigned int frame_count = 60;
static  unsigned int frame_coded = 0;
static  unsigned int frame_bitrate = 0;
static  unsigned int frame_slices = 1;
static  double frame_size = 0;
static  int initial_qp = 26;
static  int minimal_qp = 0;
static  int intra_period = 30;
static  int intra_idr_period = 60;
static  int ip_period = 1;
static  int rc_mode = -1;
static  int rc_default_modes[] = {
    VA_RC_VBR,
    VA_RC_CQP,
    VA_RC_VBR_CONSTRAINED,
    VA_RC_CBR,
    VA_RC_VCM,
    VA_RC_NONE,
};
static  unsigned long long current_frame_encoding = 0;
static  unsigned long long current_frame_display = 0;
static  unsigned long long current_IDR_display = 0;
static  unsigned int current_frame_num = 0;
static  int current_frame_type;
#define current_slot (current_frame_display % SURFACE_NUM)

static  int misc_priv_type = 0;
static  int misc_priv_value = 0;

#define MIN(a, b) ((a)>(b)?(b):(a))
#define MAX(a, b) ((a)>(b)?(a):(b))

/* thread to save coded data/upload source YUV */
struct storage_task_t {
    void *next;
    unsigned long long display_order;
    unsigned long long encode_order;
};
static  struct storage_task_t *storage_task_header = NULL, *storage_task_tail = NULL;
#define SRC_SURFACE_IN_ENCODING 0
#define SRC_SURFACE_IN_STORAGE  1
static  int srcsurface_status[SURFACE_NUM];
static  int encode_syncmode = 0;
static  pthread_mutex_t encode_mutex = PTHREAD_MUTEX_INITIALIZER;
static  pthread_cond_t  encode_cond = PTHREAD_COND_INITIALIZER;
static  pthread_t encode_thread;
    
/* for performance profiling */
static unsigned int UploadPictureTicks=0;
static unsigned int BeginPictureTicks=0;
static unsigned int RenderPictureTicks=0;
static unsigned int EndPictureTicks=0;
static unsigned int SyncPictureTicks=0;
static unsigned int SavePictureTicks=0;
static unsigned int TotalTicks=0;

#include "vaBs.h"
#include "vaMisc.h"

/*
 * Helper function for profiling purposes
 */
static unsigned int GetTickCount()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL))
        return 0;
    return tv.tv_usec/1000+tv.tv_sec*1000;
}



static int init_va(void)
{
    VAProfile profile_list[]={VAProfileH264High,VAProfileH264Main,VAProfileH264Baseline,VAProfileH264ConstrainedBaseline};
    VAEntrypoint *entrypoints;
    int num_entrypoints, slice_entrypoint;
    int support_encode = 0;    
    int major_ver, minor_ver;
    VAStatus va_status;
    unsigned int i;

    va_dpy = va_open_display();
    va_status = vaInitialize(va_dpy, &major_ver, &minor_ver);
    CHECK_VASTATUS(va_status, "vaInitialize");

    num_entrypoints = vaMaxNumEntrypoints(va_dpy);
    entrypoints = malloc(num_entrypoints * sizeof(*entrypoints));
    if (!entrypoints) {
        fprintf(stderr, "error: failed to initialize VA entrypoints array\n");
        exit(1);
    }

    /* use the highest profile */
    for (i = 0; i < sizeof(profile_list)/sizeof(profile_list[0]); i++) {
        if ((h264_profile != ~0) && h264_profile != profile_list[i])
            continue;
        
        h264_profile = profile_list[i];
        vaQueryConfigEntrypoints(va_dpy, h264_profile, entrypoints, &num_entrypoints);
        for (slice_entrypoint = 0; slice_entrypoint < num_entrypoints; slice_entrypoint++) {
            if (entrypoints[slice_entrypoint] == VAEntrypointEncSlice) {
                support_encode = 1;
                break;
            }
        }
        if (support_encode == 1)
            break;
    }
    
    if (support_encode == 0) {
        printf("Can't find VAEntrypointEncSlice for H264 profiles\n");
        exit(1);
    } else {
        switch (h264_profile) {
            case VAProfileH264Baseline:
                printf("Use profile VAProfileH264Baseline\n");
                ip_period = 1;
                constraint_set_flag |= (1 << 0); /* Annex A.2.1 */
                h264_entropy_mode = 0;
                break;
            case VAProfileH264ConstrainedBaseline:
                printf("Use profile VAProfileH264ConstrainedBaseline\n");
                constraint_set_flag |= (1 << 0 | 1 << 1); /* Annex A.2.2 */
                ip_period = 1;
                break;

            case VAProfileH264Main:
                printf("Use profile VAProfileH264Main\n");
                constraint_set_flag |= (1 << 1); /* Annex A.2.2 */
                break;

            case VAProfileH264High:
                constraint_set_flag |= (1 << 3); /* Annex A.2.4 */
                printf("Use profile VAProfileH264High\n");
                break;
            default:
                printf("unknow profile. Set to Baseline");
                h264_profile = VAProfileH264Baseline;
                ip_period = 1;
                constraint_set_flag |= (1 << 0); /* Annex A.2.1 */
                break;
        }
    }

    /* find out the format for the render target, and rate control mode */
    for (i = 0; i < VAConfigAttribTypeMax; i++)
        attrib[i].type = i;

    va_status = vaGetConfigAttributes(va_dpy, h264_profile, VAEntrypointEncSlice,
                                      &attrib[0], VAConfigAttribTypeMax);
    CHECK_VASTATUS(va_status, "vaGetConfigAttributes");
    /* check the interested configattrib */
    if ((attrib[VAConfigAttribRTFormat].value & VA_RT_FORMAT_YUV420) == 0) {
        printf("Not find desired YUV420 RT format\n");
        exit(1);
    } else {
        config_attrib[config_attrib_num].type = VAConfigAttribRTFormat;
        config_attrib[config_attrib_num].value = VA_RT_FORMAT_YUV420;
        config_attrib_num++;
    }
    
    if (attrib[VAConfigAttribRateControl].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribRateControl].value;

        printf("Support rate control mode (0x%x):", tmp);
        
        if (tmp & VA_RC_NONE)
            printf("NONE ");
        if (tmp & VA_RC_CBR)
            printf("CBR ");
        if (tmp & VA_RC_VBR)
            printf("VBR ");
        if (tmp & VA_RC_VCM)
            printf("VCM ");
        if (tmp & VA_RC_CQP)
            printf("CQP ");
        if (tmp & VA_RC_VBR_CONSTRAINED)
            printf("VBR_CONSTRAINED ");

        printf("\n");

        if (rc_mode == -1 || !(rc_mode & tmp))  {
            if (rc_mode != -1) {
                printf("Warning: Don't support the specified RateControl mode: %s!!!, switch to ", rc_to_string(rc_mode));
            }

            for (i = 0; i < sizeof(rc_default_modes) / sizeof(rc_default_modes[0]); i++) {
                if (rc_default_modes[i] & tmp) {
                    rc_mode = rc_default_modes[i];
                    break;
                }
            }

            printf("RateControl mode: %s\n", rc_to_string(rc_mode));
        }

        config_attrib[config_attrib_num].type = VAConfigAttribRateControl;
        config_attrib[config_attrib_num].value = rc_mode;
        config_attrib_num++;
    }
    

    if (attrib[VAConfigAttribEncPackedHeaders].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribEncPackedHeaders].value;

        printf("Support VAConfigAttribEncPackedHeaders\n");
        
        h264_packedheader = 1;
        config_attrib[config_attrib_num].type = VAConfigAttribEncPackedHeaders;
        config_attrib[config_attrib_num].value = VA_ENC_PACKED_HEADER_NONE;
        
        if (tmp & VA_ENC_PACKED_HEADER_SEQUENCE) {
            printf("Support packed sequence headers\n");
            config_attrib[config_attrib_num].value |= VA_ENC_PACKED_HEADER_SEQUENCE;
        }
        
        if (tmp & VA_ENC_PACKED_HEADER_PICTURE) {
            printf("Support packed picture headers\n");
            config_attrib[config_attrib_num].value |= VA_ENC_PACKED_HEADER_PICTURE;
        }
        
        if (tmp & VA_ENC_PACKED_HEADER_SLICE) {
            printf("Support packed slice headers\n");
            config_attrib[config_attrib_num].value |= VA_ENC_PACKED_HEADER_SLICE;
        }
        
        if (tmp & VA_ENC_PACKED_HEADER_MISC) {
            printf("Support packed misc headers\n");
            config_attrib[config_attrib_num].value |= VA_ENC_PACKED_HEADER_MISC;
        }
        
        enc_packed_header_idx = config_attrib_num;
        config_attrib_num++;
    }

    if (attrib[VAConfigAttribEncInterlaced].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribEncInterlaced].value;
        
        printf("Support VAConfigAttribEncInterlaced\n");

        if (tmp & VA_ENC_INTERLACED_FRAME)
            printf("support VA_ENC_INTERLACED_FRAME\n");
        if (tmp & VA_ENC_INTERLACED_FIELD)
            printf("Support VA_ENC_INTERLACED_FIELD\n");
        if (tmp & VA_ENC_INTERLACED_MBAFF)
            printf("Support VA_ENC_INTERLACED_MBAFF\n");
        if (tmp & VA_ENC_INTERLACED_PAFF)
            printf("Support VA_ENC_INTERLACED_PAFF\n");
        
        config_attrib[config_attrib_num].type = VAConfigAttribEncInterlaced;
        config_attrib[config_attrib_num].value = VA_ENC_PACKED_HEADER_NONE;
        config_attrib_num++;
    }
    
    if (attrib[VAConfigAttribEncMaxRefFrames].value != VA_ATTRIB_NOT_SUPPORTED) {
        h264_maxref = attrib[VAConfigAttribEncMaxRefFrames].value;
        
        printf("Support %d RefPicList0 and %d RefPicList1\n",
               h264_maxref & 0xffff, (h264_maxref >> 16) & 0xffff );
    }

    if (attrib[VAConfigAttribEncMaxSlices].value != VA_ATTRIB_NOT_SUPPORTED)
        printf("Support %d slices\n", attrib[VAConfigAttribEncMaxSlices].value);

    if (attrib[VAConfigAttribEncSliceStructure].value != VA_ATTRIB_NOT_SUPPORTED) {
        int tmp = attrib[VAConfigAttribEncSliceStructure].value;
        
        printf("Support VAConfigAttribEncSliceStructure\n");

        if (tmp & VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS)
            printf("Support VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS\n");
        if (tmp & VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS)
            printf("Support VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS\n");
        if (tmp & VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS)
            printf("Support VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS\n");
    }
    if (attrib[VAConfigAttribEncMacroblockInfo].value != VA_ATTRIB_NOT_SUPPORTED) {
        printf("Support VAConfigAttribEncMacroblockInfo\n");
    }

    free(entrypoints);
    return 0;
}

static int setup_encode()
{
    VAStatus va_status;
    VASurfaceID *tmp_surfaceid;
    int codedbuf_size, i;
    
    va_status = vaCreateConfig(va_dpy, h264_profile, VAEntrypointEncSlice,
            &config_attrib[0], config_attrib_num, &config_id);
    CHECK_VASTATUS(va_status, "vaCreateConfig");

    /* create source surfaces */
    va_status = vaCreateSurfaces(va_dpy,
                                 VA_RT_FORMAT_YUV420, frame_width_mbaligned, frame_height_mbaligned,
                                 &src_surface[0], SURFACE_NUM,
                                 NULL, 0);
    CHECK_VASTATUS(va_status, "vaCreateSurfaces");

    /* create reference surfaces */
    va_status = vaCreateSurfaces(
        va_dpy,
        VA_RT_FORMAT_YUV420, frame_width_mbaligned, frame_height_mbaligned,
        &ref_surface[0], SURFACE_NUM,
        NULL, 0
        );
    CHECK_VASTATUS(va_status, "vaCreateSurfaces");

    tmp_surfaceid = calloc(2 * SURFACE_NUM, sizeof(VASurfaceID));
    memcpy(tmp_surfaceid, src_surface, SURFACE_NUM * sizeof(VASurfaceID));
    memcpy(tmp_surfaceid + SURFACE_NUM, ref_surface, SURFACE_NUM * sizeof(VASurfaceID));
    
    /* Create a context for this encode pipe */
    va_status = vaCreateContext(va_dpy, config_id,
                                frame_width_mbaligned, frame_height_mbaligned,
                                VA_PROGRESSIVE,
                                tmp_surfaceid, 2 * SURFACE_NUM,
                                &context_id);
    CHECK_VASTATUS(va_status, "vaCreateContext");
    free(tmp_surfaceid);

    codedbuf_size = (frame_width_mbaligned * frame_height_mbaligned * 400) / (16*16);

    for (i = 0; i < SURFACE_NUM; i++) {
        /* create coded buffer once for all
         * other VA buffers which won't be used again after vaRenderPicture.
         * so APP can always vaCreateBuffer for every frame
         * but coded buffer need to be mapped and accessed after vaRenderPicture/vaEndPicture
         * so VA won't maintain the coded buffer
         */
        va_status = vaCreateBuffer(va_dpy,context_id,VAEncCodedBufferType,
                codedbuf_size, 1, NULL, &coded_buf[i]);
        CHECK_VASTATUS(va_status,"vaCreateBuffer");
    }
    
    return 0;
}



#define partition(ref, field, key, ascending)   \
    while (i <= j) {                            \
        if (ascending) {                        \
            while (ref[i].field < key)          \
                i++;                            \
            while (ref[j].field > key)          \
                j--;                            \
        } else {                                \
            while (ref[i].field > key)          \
                i++;                            \
            while (ref[j].field < key)          \
                j--;                            \
        }                                       \
        if (i <= j) {                           \
            tmp = ref[i];                       \
            ref[i] = ref[j];                    \
            ref[j] = tmp;                       \
            i++;                                \
            j--;                                \
        }                                       \
    }                                           \

static void sort_one(VAPictureH264 ref[], int left, int right,
                     int ascending, int frame_idx)
{
    int i = left, j = right;
    unsigned int key;
    VAPictureH264 tmp;

    if (frame_idx) {
        key = ref[(left + right) / 2].frame_idx;
        partition(ref, frame_idx, key, ascending);
    } else {
        key = ref[(left + right) / 2].TopFieldOrderCnt;
        partition(ref, TopFieldOrderCnt, (signed int)key, ascending);
    }
    
    /* recursion */
    if (left < j)
        sort_one(ref, left, j, ascending, frame_idx);
    
    if (i < right)
        sort_one(ref, i, right, ascending, frame_idx);
}

static void sort_two(VAPictureH264 ref[], int left, int right, unsigned int key, unsigned int frame_idx,
                     int partition_ascending, int list0_ascending, int list1_ascending)
{
    int i = left, j = right;
    VAPictureH264 tmp;

    if (frame_idx) {
        partition(ref, frame_idx, key, partition_ascending);
    } else {
        partition(ref, TopFieldOrderCnt, (signed int)key, partition_ascending);
    }
    

    sort_one(ref, left, i-1, list0_ascending, frame_idx);
    sort_one(ref, j+1, right, list1_ascending, frame_idx);
}

static int update_ReferenceFrames(void)
{
    int i;
    
    if (current_frame_type == FRAME_B)
        return 0;

    CurrentCurrPic.flags = VA_PICTURE_H264_SHORT_TERM_REFERENCE;
    numShortTerm++;
    if (numShortTerm > num_ref_frames)
        numShortTerm = num_ref_frames;
    for (i=numShortTerm-1; i>0; i--)
        ReferenceFrames[i] = ReferenceFrames[i-1];
    ReferenceFrames[0] = CurrentCurrPic;
    
    if (current_frame_type != FRAME_B)
        current_frame_num++;
    if (current_frame_num > MaxFrameNum)
        current_frame_num = 0;
    
    return 0;
}


static int update_RefPicList(void)
{
    unsigned int current_poc = CurrentCurrPic.TopFieldOrderCnt;
    
    if (current_frame_type == FRAME_P) {
        memcpy(RefPicList0_P, ReferenceFrames, numShortTerm * sizeof(VAPictureH264));
        sort_one(RefPicList0_P, 0, numShortTerm-1, 0, 1);
    }
    
    if (current_frame_type == FRAME_B) {
        memcpy(RefPicList0_B, ReferenceFrames, numShortTerm * sizeof(VAPictureH264));
        sort_two(RefPicList0_B, 0, numShortTerm-1, current_poc, 0,
                 1, 0, 1);

        memcpy(RefPicList1_B, ReferenceFrames, numShortTerm * sizeof(VAPictureH264));
        sort_two(RefPicList1_B, 0, numShortTerm-1, current_poc, 0,
                 0, 1, 0);
    }
    
    return 0;
}


static int render_sequence(void)
{
    VABufferID seq_param_buf, rc_param_buf, misc_param_tmpbuf, render_id[2];
    VAStatus va_status;
    VAEncMiscParameterBuffer *misc_param, *misc_param_tmp;
    VAEncMiscParameterRateControl *misc_rate_ctrl;
    
    seq_param.level_idc = 41 /*SH_LEVEL_3*/;
    seq_param.picture_width_in_mbs = frame_width_mbaligned / 16;
    seq_param.picture_height_in_mbs = frame_height_mbaligned / 16;
    seq_param.bits_per_second = frame_bitrate;

    seq_param.intra_period = intra_period;
    seq_param.intra_idr_period = intra_idr_period;
    seq_param.ip_period = ip_period;

    seq_param.max_num_ref_frames = num_ref_frames;
    seq_param.seq_fields.bits.frame_mbs_only_flag = 1;
    seq_param.time_scale = 900;
    seq_param.num_units_in_tick = 15; /* Tc = num_units_in_tick / time_sacle */
    seq_param.seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4 = Log2MaxPicOrderCntLsb - 4;
    seq_param.seq_fields.bits.log2_max_frame_num_minus4 = Log2MaxFrameNum - 4;;
    seq_param.seq_fields.bits.frame_mbs_only_flag = 1;
    seq_param.seq_fields.bits.chroma_format_idc = 1;
    seq_param.seq_fields.bits.direct_8x8_inference_flag = 1;
    
    if (frame_width != frame_width_mbaligned ||
        frame_height != frame_height_mbaligned) {
        seq_param.frame_cropping_flag = 1;
        seq_param.frame_crop_left_offset = 0;
        seq_param.frame_crop_right_offset = (frame_width_mbaligned - frame_width)/2;
        seq_param.frame_crop_top_offset = 0;
        seq_param.frame_crop_bottom_offset = (frame_height_mbaligned - frame_height)/2;
    }
    
    va_status = vaCreateBuffer(va_dpy, context_id,
                               VAEncSequenceParameterBufferType,
                               sizeof(seq_param),1,&seq_param,&seq_param_buf);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");
    
    va_status = vaCreateBuffer(va_dpy, context_id,
                               VAEncMiscParameterBufferType,
                               sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterRateControl),
                               1,NULL,&rc_param_buf);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");
    
    vaMapBuffer(va_dpy, rc_param_buf,(void **)&misc_param);
    misc_param->type = VAEncMiscParameterTypeRateControl;
    misc_rate_ctrl = (VAEncMiscParameterRateControl *)misc_param->data;
    memset(misc_rate_ctrl, 0, sizeof(*misc_rate_ctrl));
    misc_rate_ctrl->bits_per_second = frame_bitrate;
    misc_rate_ctrl->target_percentage = 66;
    misc_rate_ctrl->window_size = 1000;
    misc_rate_ctrl->initial_qp = initial_qp;
    misc_rate_ctrl->min_qp = minimal_qp;
    misc_rate_ctrl->basic_unit_size = 0;
    vaUnmapBuffer(va_dpy, rc_param_buf);

    render_id[0] = seq_param_buf;
    render_id[1] = rc_param_buf;
    
    va_status = vaRenderPicture(va_dpy,context_id, &render_id[0], 2);
    CHECK_VASTATUS(va_status,"vaRenderPicture");;

    if (misc_priv_type != 0) {
        va_status = vaCreateBuffer(va_dpy, context_id,
                                   VAEncMiscParameterBufferType,
                                   sizeof(VAEncMiscParameterBuffer),
                                   1, NULL, &misc_param_tmpbuf);
        CHECK_VASTATUS(va_status,"vaCreateBuffer");
        vaMapBuffer(va_dpy, misc_param_tmpbuf,(void **)&misc_param_tmp);
        misc_param_tmp->type = misc_priv_type;
        misc_param_tmp->data[0] = misc_priv_value;
        vaUnmapBuffer(va_dpy, misc_param_tmpbuf);
    
        va_status = vaRenderPicture(va_dpy,context_id, &misc_param_tmpbuf, 1);
    }
    
    return 0;
}

static int calc_poc(int pic_order_cnt_lsb)
{
    static int PicOrderCntMsb_ref = 0, pic_order_cnt_lsb_ref = 0;
    int prevPicOrderCntMsb, prevPicOrderCntLsb;
    int PicOrderCntMsb, TopFieldOrderCnt;
    
    if (current_frame_type == FRAME_IDR)
        prevPicOrderCntMsb = prevPicOrderCntLsb = 0;
    else {
        prevPicOrderCntMsb = PicOrderCntMsb_ref;
        prevPicOrderCntLsb = pic_order_cnt_lsb_ref;
    }
    
    if ((pic_order_cnt_lsb < prevPicOrderCntLsb) &&
        ((prevPicOrderCntLsb - pic_order_cnt_lsb) >= (int)(MaxPicOrderCntLsb / 2)))
        PicOrderCntMsb = prevPicOrderCntMsb + MaxPicOrderCntLsb;
    else if ((pic_order_cnt_lsb > prevPicOrderCntLsb) &&
             ((pic_order_cnt_lsb - prevPicOrderCntLsb) > (int)(MaxPicOrderCntLsb / 2)))
        PicOrderCntMsb = prevPicOrderCntMsb - MaxPicOrderCntLsb;
    else
        PicOrderCntMsb = prevPicOrderCntMsb;
    
    TopFieldOrderCnt = PicOrderCntMsb + pic_order_cnt_lsb;

    if (current_frame_type != FRAME_B) {
        PicOrderCntMsb_ref = PicOrderCntMsb;
        pic_order_cnt_lsb_ref = pic_order_cnt_lsb;
    }
    
    return TopFieldOrderCnt;
}

static int render_picture(void)
{
    VABufferID pic_param_buf;
    VAStatus va_status;
    int i = 0;

    pic_param.CurrPic.picture_id = ref_surface[current_slot];
    pic_param.CurrPic.frame_idx = current_frame_num;
    pic_param.CurrPic.flags = 0;
    pic_param.CurrPic.TopFieldOrderCnt = calc_poc((current_frame_display - current_IDR_display) % MaxPicOrderCntLsb);
    pic_param.CurrPic.BottomFieldOrderCnt = pic_param.CurrPic.TopFieldOrderCnt;
    CurrentCurrPic = pic_param.CurrPic;

    if (getenv("TO_DEL")) { /* set RefPicList into ReferenceFrames */
        update_RefPicList(); /* calc RefPicList */
        memset(pic_param.ReferenceFrames, 0xff, 16 * sizeof(VAPictureH264)); /* invalid all */
        if (current_frame_type == FRAME_P) {
            pic_param.ReferenceFrames[0] = RefPicList0_P[0];
        } else if (current_frame_type == FRAME_B) {
            pic_param.ReferenceFrames[0] = RefPicList0_B[0];
            pic_param.ReferenceFrames[1] = RefPicList1_B[0];
        }
    } else {
        memcpy(pic_param.ReferenceFrames, ReferenceFrames, numShortTerm*sizeof(VAPictureH264));
        for (i = numShortTerm; i < SURFACE_NUM; i++) {
            pic_param.ReferenceFrames[i].picture_id = VA_INVALID_SURFACE;
            pic_param.ReferenceFrames[i].flags = VA_PICTURE_H264_INVALID;
        }
    }
    
    pic_param.pic_fields.bits.idr_pic_flag = (current_frame_type == FRAME_IDR);
    pic_param.pic_fields.bits.reference_pic_flag = (current_frame_type != FRAME_B);
    pic_param.pic_fields.bits.entropy_coding_mode_flag = h264_entropy_mode;
    pic_param.pic_fields.bits.deblocking_filter_control_present_flag = 1;
    pic_param.frame_num = current_frame_num;
    pic_param.coded_buf = coded_buf[current_slot];
    pic_param.last_picture = (current_frame_encoding == frame_count);
    pic_param.pic_init_qp = initial_qp;

    va_status = vaCreateBuffer(va_dpy, context_id,VAEncPictureParameterBufferType,
                               sizeof(pic_param),1,&pic_param, &pic_param_buf);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");;

    va_status = vaRenderPicture(va_dpy,context_id, &pic_param_buf, 1);
    CHECK_VASTATUS(va_status,"vaRenderPicture");

    return 0;
}

static int render_packedsequence(void)
{
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedseq_para_bufid, packedseq_data_bufid, render_id[2];
    unsigned int length_in_bits;
    unsigned char *packedseq_buffer = NULL;
    VAStatus va_status;

    length_in_bits = build_packed_seq_buffer(&packedseq_buffer); 
    
    packedheader_param_buffer.type = VAEncPackedHeaderSequence;
    
    packedheader_param_buffer.bit_length = length_in_bits; /*length_in_bits*/
    packedheader_param_buffer.has_emulation_bytes = 0;
    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderParameterBufferType,
                               sizeof(packedheader_param_buffer), 1, &packedheader_param_buffer,
                               &packedseq_para_bufid);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderDataBufferType,
                               (length_in_bits + 7) / 8, 1, packedseq_buffer,
                               &packedseq_data_bufid);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    render_id[0] = packedseq_para_bufid;
    render_id[1] = packedseq_data_bufid;
    va_status = vaRenderPicture(va_dpy,context_id, render_id, 2);
    CHECK_VASTATUS(va_status,"vaRenderPicture");

    free(packedseq_buffer);
    
    return 0;
}


static int render_packedpicture(void)
{
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedpic_para_bufid, packedpic_data_bufid, render_id[2];
    unsigned int length_in_bits;
    unsigned char *packedpic_buffer = NULL;
    VAStatus va_status;

    length_in_bits = build_packed_pic_buffer(&packedpic_buffer); 
    packedheader_param_buffer.type = VAEncPackedHeaderPicture;
    packedheader_param_buffer.bit_length = length_in_bits;
    packedheader_param_buffer.has_emulation_bytes = 0;

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderParameterBufferType,
                               sizeof(packedheader_param_buffer), 1, &packedheader_param_buffer,
                               &packedpic_para_bufid);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderDataBufferType,
                               (length_in_bits + 7) / 8, 1, packedpic_buffer,
                               &packedpic_data_bufid);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    render_id[0] = packedpic_para_bufid;
    render_id[1] = packedpic_data_bufid;
    va_status = vaRenderPicture(va_dpy,context_id, render_id, 2);
    CHECK_VASTATUS(va_status,"vaRenderPicture");

    free(packedpic_buffer);
    
    return 0;
}

static void render_packedsei(void)
{
    VAEncPackedHeaderParameterBuffer packed_header_param_buffer;
    VABufferID packed_sei_header_param_buf_id, packed_sei_buf_id, render_id[2];
    unsigned int length_in_bits /*offset_in_bytes*/;
    unsigned char *packed_sei_buffer = NULL;
    VAStatus va_status;
    int init_cpb_size, target_bit_rate, i_initial_cpb_removal_delay_length, i_initial_cpb_removal_delay;
    int i_cpb_removal_delay, i_dpb_output_delay_length, i_cpb_removal_delay_length;

    /* it comes for the bps defined in SPS */
    target_bit_rate = frame_bitrate;
    init_cpb_size = (target_bit_rate * 8) >> 10;
    i_initial_cpb_removal_delay = init_cpb_size * 0.5 * 1024 / target_bit_rate * 90000;

    i_cpb_removal_delay = 2;
    i_initial_cpb_removal_delay_length = 24;
    i_cpb_removal_delay_length = 24;
    i_dpb_output_delay_length = 24;
    

    length_in_bits = build_packed_sei_buffer_timing(
        i_initial_cpb_removal_delay_length,
        i_initial_cpb_removal_delay,
        0,
        i_cpb_removal_delay_length,
        i_cpb_removal_delay * current_frame_encoding,
        i_dpb_output_delay_length,
        0,
        &packed_sei_buffer);

    //offset_in_bytes = 0;
    packed_header_param_buffer.type = VAEncPackedHeaderH264_SEI;
    packed_header_param_buffer.bit_length = length_in_bits;
    packed_header_param_buffer.has_emulation_bytes = 0;

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderParameterBufferType,
                               sizeof(packed_header_param_buffer), 1, &packed_header_param_buffer,
                               &packed_sei_header_param_buf_id);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderDataBufferType,
                               (length_in_bits + 7) / 8, 1, packed_sei_buffer,
                               &packed_sei_buf_id);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");


    render_id[0] = packed_sei_header_param_buf_id;
    render_id[1] = packed_sei_buf_id;
    va_status = vaRenderPicture(va_dpy,context_id, render_id, 2);
    CHECK_VASTATUS(va_status,"vaRenderPicture");

    
    free(packed_sei_buffer);
        
    return;
}


static int render_hrd(void)
{
    VABufferID misc_parameter_hrd_buf_id;
    VAStatus va_status;
    VAEncMiscParameterBuffer *misc_param;
    VAEncMiscParameterHRD *misc_hrd_param;
    
    va_status = vaCreateBuffer(va_dpy, context_id,
                   VAEncMiscParameterBufferType,
                   sizeof(VAEncMiscParameterBuffer) + sizeof(VAEncMiscParameterHRD),
                   1,
                   NULL, 
                   &misc_parameter_hrd_buf_id);
    CHECK_VASTATUS(va_status, "vaCreateBuffer");

    vaMapBuffer(va_dpy,
                misc_parameter_hrd_buf_id,
                (void **)&misc_param);
    misc_param->type = VAEncMiscParameterTypeHRD;
    misc_hrd_param = (VAEncMiscParameterHRD *)misc_param->data;

    if (frame_bitrate > 0) {
        misc_hrd_param->initial_buffer_fullness = frame_bitrate * 1024 * 4;
        misc_hrd_param->buffer_size = frame_bitrate * 1024 * 8;
    } else {
        misc_hrd_param->initial_buffer_fullness = 0;
        misc_hrd_param->buffer_size = 0;
    }
    vaUnmapBuffer(va_dpy, misc_parameter_hrd_buf_id);

    va_status = vaRenderPicture(va_dpy,context_id, &misc_parameter_hrd_buf_id, 1);
    CHECK_VASTATUS(va_status,"vaRenderPicture");;

    return 0;
}

static void render_packedslice()
{
    VAEncPackedHeaderParameterBuffer packedheader_param_buffer;
    VABufferID packedslice_para_bufid, packedslice_data_bufid, render_id[2];
    unsigned int length_in_bits;
    unsigned char *packedslice_buffer = NULL;
    VAStatus va_status;

    length_in_bits = build_packed_slice_buffer(&packedslice_buffer);
    packedheader_param_buffer.type = VAEncPackedHeaderSlice;
    packedheader_param_buffer.bit_length = length_in_bits;
    packedheader_param_buffer.has_emulation_bytes = 0;

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderParameterBufferType,
                               sizeof(packedheader_param_buffer), 1, &packedheader_param_buffer,
                               &packedslice_para_bufid);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    va_status = vaCreateBuffer(va_dpy,
                               context_id,
                               VAEncPackedHeaderDataBufferType,
                               (length_in_bits + 7) / 8, 1, packedslice_buffer,
                               &packedslice_data_bufid);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");

    render_id[0] = packedslice_para_bufid;
    render_id[1] = packedslice_data_bufid;
    va_status = vaRenderPicture(va_dpy,context_id, render_id, 2);
    CHECK_VASTATUS(va_status,"vaRenderPicture");

    free(packedslice_buffer);
}

static int render_slice(void)
{
    VABufferID slice_param_buf;
    VAStatus va_status;
    int i;

    update_RefPicList();
    
    /* one frame, one slice */
    slice_param.macroblock_address = 0;
    slice_param.num_macroblocks = frame_width_mbaligned * frame_height_mbaligned/(16*16); /* Measured by MB */
    slice_param.slice_type = (current_frame_type == FRAME_IDR)?2:current_frame_type;
    if (current_frame_type == FRAME_IDR) {
        if (current_frame_encoding != 0)
            ++slice_param.idr_pic_id;
    } else if (current_frame_type == FRAME_P) {
        int refpiclist0_max = h264_maxref & 0xffff;
        memcpy(slice_param.RefPicList0, RefPicList0_P, refpiclist0_max*sizeof(VAPictureH264));

        for (i = refpiclist0_max; i < 32; i++) {
            slice_param.RefPicList0[i].picture_id = VA_INVALID_SURFACE;
            slice_param.RefPicList0[i].flags = VA_PICTURE_H264_INVALID;
        }
    } else if (current_frame_type == FRAME_B) {
        int refpiclist0_max = h264_maxref & 0xffff;
        int refpiclist1_max = (h264_maxref >> 16) & 0xffff;

        memcpy(slice_param.RefPicList0, RefPicList0_B, refpiclist0_max*sizeof(VAPictureH264));
        for (i = refpiclist0_max; i < 32; i++) {
            slice_param.RefPicList0[i].picture_id = VA_INVALID_SURFACE;
            slice_param.RefPicList0[i].flags = VA_PICTURE_H264_INVALID;
        }

        memcpy(slice_param.RefPicList1, RefPicList1_B, refpiclist1_max*sizeof(VAPictureH264));
        for (i = refpiclist1_max; i < 32; i++) {
            slice_param.RefPicList1[i].picture_id = VA_INVALID_SURFACE;
            slice_param.RefPicList1[i].flags = VA_PICTURE_H264_INVALID;
        }
    }

    slice_param.slice_alpha_c0_offset_div2 = 0;
    slice_param.slice_beta_offset_div2 = 0;
    slice_param.direct_spatial_mv_pred_flag = 1;
    slice_param.pic_order_cnt_lsb = (current_frame_display - current_IDR_display) % MaxPicOrderCntLsb;
    

    if (h264_packedheader &&
        config_attrib[enc_packed_header_idx].value & VA_ENC_PACKED_HEADER_SLICE)
        render_packedslice();

    va_status = vaCreateBuffer(va_dpy,context_id,VAEncSliceParameterBufferType,
                               sizeof(slice_param),1,&slice_param,&slice_param_buf);
    CHECK_VASTATUS(va_status,"vaCreateBuffer");;

    va_status = vaRenderPicture(va_dpy,context_id, &slice_param_buf, 1);
    CHECK_VASTATUS(va_status,"vaRenderPicture");
    
    return 0;
}


static int upload_source_YUV_once_for_all()
{
    int box_width=8;
    int row_shift=0;
    int i;

    for (i = 0; i < SURFACE_NUM; i++) {
        printf("\rLoading data into surface %d.....", i);
        upload_surface(va_dpy, src_surface[i], box_width, row_shift, 0);

        row_shift++;
        if (row_shift==(2*box_width)) row_shift= 0;
    }
    printf("Complete surface loading\n");

    return 0;
}

static int load_surface(VASurfaceID surface_id, unsigned long long display_order)
{
    unsigned char *srcyuv_ptr = NULL, *src_Y = NULL, *src_U = NULL, *src_V = NULL;
    unsigned long long frame_start, mmap_start;
    char *mmap_ptr = NULL;
    int frame_size, mmap_size;
    
    if (srcyuv_fp == NULL)
        return 0;
    
    /* allow encoding more than srcyuv_frames */    
    frame_size = frame_width * frame_height * 3 / 2; /* for YUV420 */
    frame_start = display_order * frame_size;
    
    mmap_start = frame_start & (~0xfff);
    mmap_size = (frame_size + (frame_start & 0xfff) + 0xfff) & (~0xfff);
    mmap_ptr = mmap(0, mmap_size, PROT_READ, MAP_SHARED,
                    fileno(srcyuv_fp), mmap_start);
    if (mmap_ptr == MAP_FAILED) {
        printf("Failed to mmap YUV file (%s)\n", strerror(errno));
        return 1;
    }
    srcyuv_ptr = (unsigned char *)mmap_ptr +  (frame_start & 0xfff);
    if (srcyuv_fourcc == VA_FOURCC_NV12) {
        src_Y = srcyuv_ptr;
        src_U = src_Y + frame_width * frame_height;
        src_V = NULL;
    } else if (srcyuv_fourcc == VA_FOURCC_IYUV ||
        srcyuv_fourcc == VA_FOURCC_YV12) {
        src_Y = srcyuv_ptr;
        if (srcyuv_fourcc == VA_FOURCC_IYUV) {
            src_U = src_Y + frame_width * frame_height;
            src_V = src_U + (frame_width/2) * (frame_height/2);
        } else { /* YV12 */
            src_V = src_Y + frame_width * frame_height;
            src_U = src_V + (frame_width/2) * (frame_height/2);
        } 
    } else {
        printf("Unsupported source YUV format\n");
        exit(1);
    }
    
    upload_surface_yuv(va_dpy, surface_id,
                       srcyuv_fourcc, frame_width, frame_height,
                       src_Y, src_U, src_V);
    if (mmap_ptr)
        munmap(mmap_ptr, mmap_size);

    return 0;
}


static int save_recyuv(VASurfaceID surface_id,
                       unsigned long long display_order,
                       unsigned long long encode_order)
{
    unsigned char *dst_Y = NULL, *dst_U = NULL, *dst_V = NULL;

    if (recyuv_fp == NULL)
        return 0;

    if (srcyuv_fourcc == VA_FOURCC_NV12) {
        int uv_size = 2 * (frame_width/2) * (frame_height/2);
        dst_Y = malloc(2*uv_size);
        dst_U = malloc(uv_size);
    } else if (srcyuv_fourcc == VA_FOURCC_IYUV ||
               srcyuv_fourcc == VA_FOURCC_YV12) {
        int uv_size = (frame_width/2) * (frame_height/2);
        dst_Y = malloc(4*uv_size);
        dst_U = malloc(uv_size);
        dst_V = malloc(uv_size);
    } else {
        printf("Unsupported source YUV format\n");
        exit(1);
    }
    
    download_surface_yuv(va_dpy, surface_id,
                         srcyuv_fourcc, frame_width, frame_height,
                         dst_Y, dst_U, dst_V);
    fseek(recyuv_fp, display_order * frame_width * frame_height * 1.5, SEEK_SET);

    if (srcyuv_fourcc == VA_FOURCC_NV12) {
        int uv_size = 2 * (frame_width/2) * (frame_height/2);
        fwrite(dst_Y, uv_size * 2, 1, recyuv_fp);
        fwrite(dst_U, uv_size, 1, recyuv_fp);
    } else if (srcyuv_fourcc == VA_FOURCC_IYUV ||
               srcyuv_fourcc == VA_FOURCC_YV12) {
        int uv_size = (frame_width/2) * (frame_height/2);
        fwrite(dst_Y, uv_size * 4, 1, recyuv_fp);
        
        if (srcyuv_fourcc == VA_FOURCC_IYUV) {
            fwrite(dst_U, uv_size, 1, recyuv_fp);
            fwrite(dst_V, uv_size, 1, recyuv_fp);
        } else {
            fwrite(dst_V, uv_size, 1, recyuv_fp);
            fwrite(dst_U, uv_size, 1, recyuv_fp);
        }
    } else {
        printf("Unsupported YUV format\n");
        exit(1);
    }
    
    if (dst_Y)
        free(dst_Y);
    if (dst_U)
        free(dst_U);
    if (dst_V)
        free(dst_V);

    fflush(recyuv_fp);

    return 0;
}


static int save_codeddata(unsigned long long display_order, unsigned long long encode_order)
{    
    VACodedBufferSegment *buf_list = NULL;
    VAStatus va_status;
    unsigned int coded_size = 0;

    va_status = vaMapBuffer(va_dpy,coded_buf[display_order % SURFACE_NUM],(void **)(&buf_list));
    CHECK_VASTATUS(va_status,"vaMapBuffer");
    while (buf_list != NULL) {
        coded_size += fwrite(buf_list->buf, 1, buf_list->size, coded_fp);
        buf_list = (VACodedBufferSegment *) buf_list->next;

        frame_size += coded_size;
    }
    vaUnmapBuffer(va_dpy,coded_buf[display_order % SURFACE_NUM]);

    printf("\r      "); /* return back to startpoint */
    switch (encode_order % 4) {
        case 0:
            printf("|");
            break;
        case 1:
            printf("/");
            break;
        case 2:
            printf("-");
            break;
        case 3:
            printf("\\");
            break;
    }
    printf("%08lld", encode_order);
    printf("(%06d bytes coded)",coded_size);

    fflush(coded_fp);
    
    return 0;
}


static struct storage_task_t * storage_task_dequeue(void)
{
    struct storage_task_t *header;

    pthread_mutex_lock(&encode_mutex);

    header = storage_task_header;    
    if (storage_task_header != NULL) {
        if (storage_task_tail == storage_task_header)
            storage_task_tail = NULL;
        storage_task_header = header->next;
    }
    
    pthread_mutex_unlock(&encode_mutex);
    
    return header;
}

static int storage_task_queue(unsigned long long display_order, unsigned long long encode_order)
{
    struct storage_task_t *tmp;

    tmp = calloc(1, sizeof(struct storage_task_t));
    tmp->display_order = display_order;
    tmp->encode_order = encode_order;

    pthread_mutex_lock(&encode_mutex);
    
    if (storage_task_header == NULL) {
        storage_task_header = tmp;
        storage_task_tail = tmp;
    } else {
        storage_task_tail->next = tmp;
        storage_task_tail = tmp;
    }

    srcsurface_status[display_order % SURFACE_NUM] = SRC_SURFACE_IN_STORAGE;
    pthread_cond_signal(&encode_cond);
    
    pthread_mutex_unlock(&encode_mutex);
    
    return 0;
}

static void storage_task(unsigned long long display_order, unsigned long long encode_order)
{
    unsigned int tmp;
    VAStatus va_status;
    
    tmp = GetTickCount();
    va_status = vaSyncSurface(va_dpy, src_surface[display_order % SURFACE_NUM]);
    CHECK_VASTATUS(va_status,"vaSyncSurface");
    SyncPictureTicks += GetTickCount() - tmp;
    tmp = GetTickCount();
    save_codeddata(display_order, encode_order);
    SavePictureTicks += GetTickCount() - tmp;

    save_recyuv(ref_surface[display_order % SURFACE_NUM], display_order, encode_order);

    /* reload a new frame data */
    tmp = GetTickCount();
    if (srcyuv_fp != NULL)
        load_surface(src_surface[display_order % SURFACE_NUM], display_order + SURFACE_NUM);
    UploadPictureTicks += GetTickCount() - tmp;

    pthread_mutex_lock(&encode_mutex);
    srcsurface_status[display_order % SURFACE_NUM] = SRC_SURFACE_IN_ENCODING;
    pthread_mutex_unlock(&encode_mutex);
}

        
static void * storage_task_thread(void *t)
{
    while (1) {
        struct storage_task_t *current;
        
        current = storage_task_dequeue();
        if (current == NULL) {
            pthread_mutex_lock(&encode_mutex);
            pthread_cond_wait(&encode_cond, &encode_mutex);
            pthread_mutex_unlock(&encode_mutex);
            continue;
        }
        
        storage_task(current->display_order, current->encode_order);
        
        free(current);

        /* all frames are saved, exit the thread */
        if (++frame_coded >= frame_count)
            break;
    }

    return 0;
}


static int encode_frames(void)
{
    unsigned int i, tmp;
    VAStatus va_status;
    //VASurfaceStatus surface_status;

    /* upload RAW YUV data into all surfaces */
    tmp = GetTickCount();
    if (srcyuv_fp != NULL) {
        for (i = 0; i < SURFACE_NUM; i++)
            load_surface(src_surface[i], i);
    } else
        upload_source_YUV_once_for_all();
    UploadPictureTicks += GetTickCount() - tmp;
    
    /* ready for encoding */
    memset(srcsurface_status, SRC_SURFACE_IN_ENCODING, sizeof(srcsurface_status));
    
    memset(&seq_param, 0, sizeof(seq_param));
    memset(&pic_param, 0, sizeof(pic_param));
    memset(&slice_param, 0, sizeof(slice_param));

    if (encode_syncmode == 0)
        pthread_create(&encode_thread, NULL, storage_task_thread, NULL);
    
    for (current_frame_encoding = 0; current_frame_encoding < frame_count; current_frame_encoding++) {
        encoding2display_order(current_frame_encoding, intra_period, intra_idr_period, ip_period,
                               &current_frame_display, &current_frame_type);
        if (current_frame_type == FRAME_IDR) {
            numShortTerm = 0;
            current_frame_num = 0;
            current_IDR_display = current_frame_display;
        }

        /* check if the source frame is ready */
        while (srcsurface_status[current_slot] != SRC_SURFACE_IN_ENCODING) {
            usleep(1);
        }
        
        tmp = GetTickCount();
        va_status = vaBeginPicture(va_dpy, context_id, src_surface[current_slot]);
        CHECK_VASTATUS(va_status,"vaBeginPicture");
        BeginPictureTicks += GetTickCount() - tmp;
        
        tmp = GetTickCount();
        if (current_frame_type == FRAME_IDR) {
            render_sequence();
            render_picture();            
            if (h264_packedheader) {
                render_packedsequence();
                render_packedpicture();
            }
            //if (rc_mode == VA_RC_CBR)
            //    render_packedsei();
            //render_hrd();
        } else {
            //render_sequence();
            render_picture();
            //if (rc_mode == VA_RC_CBR)
            //    render_packedsei();
            //render_hrd();
        }
        render_slice();
        RenderPictureTicks += GetTickCount() - tmp;
        
        tmp = GetTickCount();
        va_status = vaEndPicture(va_dpy,context_id);
        CHECK_VASTATUS(va_status,"vaEndPicture");;
        EndPictureTicks += GetTickCount() - tmp;

        if (encode_syncmode)
            storage_task(current_frame_display, current_frame_encoding);
        else /* queue the storage task queue */
            storage_task_queue(current_frame_display, current_frame_encoding);
        
        update_ReferenceFrames();        
    }

    if (encode_syncmode == 0) {
        int ret;
        pthread_join(encode_thread, (void **)&ret);
    }
    
    return 0;
}


static int release_encode()
{
    int i;
    
    vaDestroySurfaces(va_dpy,&src_surface[0],SURFACE_NUM);
    vaDestroySurfaces(va_dpy,&ref_surface[0],SURFACE_NUM);

    for (i = 0; i < SURFACE_NUM; i++)
        vaDestroyBuffer(va_dpy,coded_buf[i]);
    
    vaDestroyContext(va_dpy,context_id);
    vaDestroyConfig(va_dpy,config_id);

    return 0;
}

static int deinit_va()
{ 
    vaTerminate(va_dpy);

    va_close_display(va_dpy);

    return 0;
}


static int print_input()
{
    printf("\n\nINPUT:Try to encode H264...\n");
    if (rc_mode != -1)
        printf("INPUT: RateControl  : %s\n", rc_to_string(rc_mode));
    printf("INPUT: Resolution   : %dx%d, %d frames\n",
           frame_width, frame_height, frame_count);
    printf("INPUT: FrameRate    : %d\n", frame_rate);
    printf("INPUT: Bitrate      : %d\n", frame_bitrate);
    printf("INPUT: Slieces      : %d\n", frame_slices);
    printf("INPUT: IntraPeriod  : %d\n", intra_period);
    printf("INPUT: IDRPeriod    : %d\n", intra_idr_period);
    printf("INPUT: IpPeriod     : %d\n", ip_period);
    printf("INPUT: Initial QP   : %d\n", initial_qp);
    printf("INPUT: Min QP       : %d\n", minimal_qp);
    printf("INPUT: Source YUV   : %s", srcyuv_fp?"FILE":"AUTO generated");
    if (srcyuv_fp) 
        printf(":%s (fourcc %s)\n", srcyuv_fn, fourcc_to_string(srcyuv_fourcc));
    else
        printf("\n");
    printf("INPUT: Coded Clip   : %s\n", coded_fn);
    if (recyuv_fp == NULL)
        printf("INPUT: Rec   Clip   : %s\n", "Not save reconstructed frame");
    else
        printf("INPUT: Rec   Clip   : Save reconstructed frame into %s (fourcc %s)\n", recyuv_fn,
               fourcc_to_string(srcyuv_fourcc));
    
    printf("\n\n"); /* return back to startpoint */
    
    return 0;
}

static int calc_PSNR(double *psnr)
{
    char *srcyuv_ptr = NULL, *recyuv_ptr = NULL, tmp;
    unsigned long long min_size;
    unsigned long long i, sse=0;
    double ssemean;
    int fourM = 0x400000; /* 4M */

    min_size = MIN(srcyuv_frames, frame_count) * frame_width * frame_height * 1.5;
    for (i=0; i<min_size; i++) {
        unsigned long long j = i % fourM;
        
        if ((i % fourM) == 0) {
            if (srcyuv_ptr)
                munmap(srcyuv_ptr, fourM);
            if (recyuv_ptr)
                munmap(recyuv_ptr, fourM);
            
            srcyuv_ptr = mmap(0, fourM, PROT_READ, MAP_SHARED, fileno(srcyuv_fp), i);
            recyuv_ptr = mmap(0, fourM, PROT_READ, MAP_SHARED, fileno(recyuv_fp), i);
            if ((srcyuv_ptr == MAP_FAILED) || (recyuv_ptr == MAP_FAILED)) {
                printf("Failed to mmap YUV files\n");
                return 1;
            }
        }
        tmp = srcyuv_ptr[j] - recyuv_ptr[j];
        sse += tmp * tmp;
    }
    ssemean = (double)sse/(double)min_size;
    *psnr = 20.0*log10(255) - 10.0*log10(ssemean);

    if (srcyuv_ptr)
        munmap(srcyuv_ptr, fourM);
    if (recyuv_ptr)
        munmap(recyuv_ptr, fourM);
    
    return 0;
}

static int print_performance(unsigned int PictureCount)
{
    unsigned int psnr_ret = 1, others = 0;
    double psnr = 0, total_size = frame_width * frame_height * 1.5 * frame_count;

    if (calc_psnr && srcyuv_fp && recyuv_fp)
        psnr_ret = calc_PSNR(&psnr);
    
    others = TotalTicks - UploadPictureTicks - BeginPictureTicks
        - RenderPictureTicks - EndPictureTicks - SyncPictureTicks - SavePictureTicks;

    printf("\n\n");

    printf("PERFORMANCE:   Frame Rate           : %.2f fps (%d frames, %d ms (%.2f ms per frame))\n",
           (double) 1000*PictureCount / TotalTicks, PictureCount,
           TotalTicks, ((double)  TotalTicks) / (double) PictureCount);
    printf("PERFORMANCE:   Compression ratio    : %d:1\n", (unsigned int)(total_size / frame_size));
    if (psnr_ret == 0)
        printf("PERFORMANCE:   PSNR                 : %.2f (%lld frames calculated)\n",
               psnr, MIN(frame_count, srcyuv_frames));

    printf("PERFORMANCE:     UploadPicture      : %d ms (%.2f, %.2f%% percent)\n",
           (int) UploadPictureTicks, ((double)  UploadPictureTicks) / (double) PictureCount,
           UploadPictureTicks/(double) TotalTicks/0.01);
    printf("PERFORMANCE:     vaBeginPicture     : %d ms (%.2f, %.2f%% percent)\n",
           (int) BeginPictureTicks, ((double)  BeginPictureTicks) / (double) PictureCount,
           BeginPictureTicks/(double) TotalTicks/0.01);
    printf("PERFORMANCE:     vaRenderHeader     : %d ms (%.2f, %.2f%% percent)\n",
           (int) RenderPictureTicks, ((double)  RenderPictureTicks) / (double) PictureCount,
           RenderPictureTicks/(double) TotalTicks/0.01);
    printf("PERFORMANCE:     vaEndPicture       : %d ms (%.2f, %.2f%% percent)\n",
           (int) EndPictureTicks, ((double)  EndPictureTicks) / (double) PictureCount,
           EndPictureTicks/(double) TotalTicks/0.01);
    printf("PERFORMANCE:     vaSyncSurface      : %d ms (%.2f, %.2f%% percent)\n",
           (int) SyncPictureTicks, ((double) SyncPictureTicks) / (double) PictureCount,
           SyncPictureTicks/(double) TotalTicks/0.01);
    printf("PERFORMANCE:     SavePicture        : %d ms (%.2f, %.2f%% percent)\n",
           (int) SavePictureTicks, ((double)  SavePictureTicks) / (double) PictureCount,
           SavePictureTicks/(double) TotalTicks/0.01);
    printf("PERFORMANCE:     Others             : %d ms (%.2f, %.2f%% percent)\n",
           (int) others, ((double) others) / (double) PictureCount,
           others/(double) TotalTicks/0.01);

    if (encode_syncmode == 0)
        printf("(Multithread enabled, the timing is only for reference)\n");
    
    return 0;
}


int main(int argc,char **argv)
{
    unsigned int start;
    
    process_cmdline(argc, argv);

    print_input();
    
    start = GetTickCount();
    
    init_va();
    setup_encode();
    
    encode_frames();

    release_encode();
    deinit_va();

    TotalTicks += GetTickCount() - start;
    print_performance(frame_count);
    
    return 0;
}
