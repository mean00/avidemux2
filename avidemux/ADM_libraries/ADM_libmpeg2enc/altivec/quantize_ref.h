/* quantize_ref.h, Low-level Architecture neutral quantization /
 * inverse quantization routines */

/*  (C) 2003 Andrew Stevens */

/* These modifications are free software; you can redistribute it
 *  and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 */

#ifndef _QUANTIZE_H
#define _QUANTIZE_H

#include "config.h"
#include "mjpeg_types.h"

struct QuantizerWorkSpace;

struct QuantizerCalls
{
    int (*pquant_non_intra)( struct QuantizerWorkSpace *wsp,
                             int16_t *src, int16_t *dst,
                             int q_scale_type, 
                             int dctsatlim,
                             int *nonsat_mquant);
    int (*pquant_weight_coeff_intra)(struct QuantizerWorkSpace *wsp,
                                     int16_t *blk );
    int (*pquant_weight_coeff_inter)(struct QuantizerWorkSpace  *wsp,
                                     int16_t *blk );

    void (*piquant_non_intra)(struct QuantizerWorkSpace  *wsp,
                              int16_t *src, int16_t *dst, int mquant );
    void (*piquant_intra)(struct QuantizerWorkSpace  *wsp,
                          int16_t *src, int16_t *dst, int dc_prec, int mquant );

};


#ifdef  __cplusplus
extern "C" {
#endif

void init_quantizer( struct QuantizerCalls *qcalls, 
                     struct QuantizerWorkSpace **workspace,
                     int mpeg1, 
                     uint16_t intra_q[64], 
                     uint16_t inter_q[64]);

void shutdown_quantizer(struct QuantizerWorkSpace *workspace);

int next_larger_quant( int q_scale_type, int quant );
int quant_code(  int q_scale_type, int mquant );
void quant_intra( struct QuantizerWorkSpace *wsp,
                  int16_t *src, int16_t *dst, 
				  int q_scale_type, int dc_prec,
                  int dctsatlim,
				  int *nonsat_mquant);
int quant_non_intra( struct QuantizerWorkSpace *wsp,
                     int16_t *src, int16_t *dst,
					 int q_scale_type,
                     int dctsatlim,
					 int *nonsat_mquant);
void iquant_intra_m1 ( struct QuantizerWorkSpace *wsp,
                       int16_t *src, int16_t *dst, int dc_prec, int mquant);
void iquant_intra_m2 ( struct QuantizerWorkSpace *wsp,
                       int16_t *src, int16_t *dst, int dc_prec, int mquant);
void iquant_non_intra_m1 (struct QuantizerWorkSpace *wsp,
                          int16_t *src, int16_t *dst, int mquant);
void iquant_non_intra_m2 (struct QuantizerWorkSpace *wsp,
                          int16_t *src, int16_t *dst, int mquant);




#ifdef  __cplusplus
}
#endif

#endif /*  _QUANTIZE_H */

/* 
 * Local variables:
 *  c-file-style: "stroustrup"
 *  tab-width: 4
 *  indent-tabs-mode: nil
 * End:
 */
