/***************************************************************************
                          ADM_extraDataExtractor
                           --------------------
       Wrapper for extract_extradata bitstream filter from libavcodec
**************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "fourcc.h"
#include "ADM_videoInfoExtractor.h"
#include "ADM_codecType.h"
#include "ADM_coreUtils.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

/**
    \fn         ADM_extractVideoExtraData
    \brief      Try to extract extradata, usually from a keyframe of size len
                at src. On success, extradata is copied to allocated buffer
                and the pointer is written to caller-provided location dest.
                The caller must delete [] the buffer to free memory.
    \return     The length of extracted extradata or a negative value on error.
*/
int ADM_extractVideoExtraData(uint32_t fcc, uint32_t len, uint8_t *src, uint8_t **dest)
{
    int r = -1;
    uint8_t *sideData = NULL;
    const AVBitStreamFilter *bsf = NULL;
    AVBSFContext *ctx = NULL;
    AVPacket *spkt = NULL, *dpkt = NULL;

    AVCodecID cid = AV_CODEC_ID_NONE;
    if(fourCC::check(fcc,(const uint8_t *)"av01"))
        cid = AV_CODEC_ID_AV1;
    else if(isH264Compatible(fcc))
        cid = AV_CODEC_ID_H264;
    else if(isH265Compatible(fcc))
        cid = AV_CODEC_ID_HEVC;
    else if(isVC1Compatible(fcc))
        cid = AV_CODEC_ID_VC1;
    /* the filter supports more codecs, the list can be extended if needed */

    if(cid == AV_CODEC_ID_NONE)
    {
        ADM_warning("Unsupported fourCC %" PRIu32" (\"%s\")\n",fcc,fourCC::tostring(fcc));
        return r;
    }

    bsf = av_bsf_get_by_name("extract_extradata");

    if(!bsf)
    {
        ADM_warning("extract_extradata bitstream filter not found.\n");
        goto _cleanup;
    }

    r = av_bsf_alloc(bsf, &ctx);

    if(r < 0)
    {
        char er[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (\"%s\") allocating AVBSFContext.\n", r, er);
        goto _cleanup;
    }

    ctx->par_in->codec_id = cid;
    ctx->par_in->codec_tag = fcc;

    r = av_bsf_init(ctx);

    if(r < 0)
    {
        char er[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (\"%s\") initiating bitstream filter.\n", r, er);
        goto _cleanup;
    }

    spkt = av_packet_alloc();
    dpkt = av_packet_alloc();

    if(!spkt || !dpkt)
    {
        ADM_error("Cannot allocate packets.\n");
        goto _cleanup;
    }

    spkt->data = src;
    spkt->size = len;
    spkt->flags = AV_PKT_FLAG_KEY;

    r = av_bsf_send_packet(ctx, spkt);

    if(r < 0)
    {
        char er[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (\"%s\") submitting data to bitstream filter.\n", r, er);
        goto _cleanup;
    }

    r = av_bsf_receive_packet(ctx, dpkt);

    if(r < 0)
    {
        char er[AV_ERROR_MAX_STRING_SIZE] = {0};
        av_make_error_string(er, AV_ERROR_MAX_STRING_SIZE, r);
        ADM_error("Error %d (\"%s\") retrieving data from bitstream filter.\n", r, er);
        goto _cleanup;
    }

    sideData = av_packet_get_side_data(dpkt, AV_PKT_DATA_NEW_EXTRADATA, &r);

    if(!sideData)
    {
        ADM_warning("No extradata extracted\n");
        goto _cleanup;
    }

    ADM_info("Extracted %d bytes of extradata\n", r);
    mixDump(sideData, r);

    *dest = new uint8_t[r];
    memcpy(*dest, sideData, r);

_cleanup:
    if(dpkt)
        av_packet_free(&dpkt);
    if(spkt)
        av_packet_free(&spkt);
    if(ctx)
        av_bsf_free(&ctx);
    return r;
}

//EOF
