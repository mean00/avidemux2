/***************************************************************************
                      NAL TYPE for H264
                      
**************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef ADM_H264_TAG_H
#define ADM_H264_TAG_H
#define NAL_NON_IDR       1
#define NAL_IDR           5
#define NAL_SEI           6
#define NAL_SPS           7
#define NAL_PPS           8
#define NAL_AU_DELIMITER  9
#define NAL_FILLER        12

typedef struct AspectRatio {
    int num;
    int den;
} AspectRatio;

const AspectRatio pixel_aspect[17] = {
	{0, 1},
	{1, 1},
	{12, 11},
	{10, 11},
	{16, 11},
	{40, 33},
	{24, 11},
	{20, 11},
	{32, 11},
	{80, 33},
	{18, 11},
	{15, 11},
	{64, 33},
	{160,99},
	{4, 3},
	{3, 2},
	{2, 1},
};
#endif
//EOF
