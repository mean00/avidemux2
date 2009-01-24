/* sub_mean_reduction_ppc.S, this file is part of the
 * AltiVec optimized library for MJPEG tools MPEG-1/2 Video Encoder
 * Copyright (C) 2002  James Klicman <james@klicman.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* register, local label and global macros */
#ifdef __linux__
#define R(register_number) register_number
#define L(label_name) .L##label_name
#define G(global_name) global_name
#else
/* Mac OS X */
#define R(register_number) r##register_number
#define L(label_name) L##label_name
#define G(global_name) _##global_name
#endif

#ifdef __linux__
	.section	".text"
#endif
	.align 2

#undef AMBER_ENABLE
#ifdef AMBER_ENABLE
amber_signal:
    mfspr R(0), 1023
    blr
#endif



	.globl	G(sub_mean_reduction_ppc)
#ifdef __linux__
	.type	G(sub_mean_reduction_ppc),@function
#endif
	
    /* arguments:
     * R(3): int len (matchset->len)
     * R(4): me_result_set* matchset
     * R(5): int reduction_times
     *
     * variables:
     * R(0):  tmp
     * R(6):  weight_sum
     * R(7):  match
     * R(8):  mean_weight
     * R(9):  tail
     * R(10): w0
     * R(11): w1
     * R(12): m0
     * R(14): m1
     * R(15): len/2
     * R(16): lr #if ALTIVEC_ENABLE
     */
G(sub_mean_reduction_ppc):
#ifdef AMBER_ENABLE
    stw     R(16),-12(R(1))  /* save nonvolatile register r16 */
    mflr    R(16)
    bl      amber_signal
#endif

    stw     R(14),-4(R(1))  /* save nonvolatile register r14 */
    cmpwi   cr1,R(3),2	    /* cmp len 2  */
    stw     R(15),-8(R(1))  /* save nonvolatile register r15 */
    mr	    R(7),R(4)	    /* match = matchset, ((++match) == matchset->mests) */
    li	    R(6),0	    /* weight_sum = 0 */
    blt	    cr1,L(sumLt2)

    srawi   R(15),R(3),1    /* len/2     */
    mtctr   R(15)	    /* ctr = len/2 */
L(sumLoop):
    lhzu    R(10),4(R(7))   /* w0 = *(++match).weight */
    lhzu    R(11),4(R(7))   /* w1 = *(++match).weight */
    add	    R(6),R(10),R(6) /* weight_sum += w0 */
    add	    R(6),R(11),R(6) /* weight_sum += w1 */
    bdnz    L(sumLoop)
L(sumLt2):
    andi.   R(0),R(3),1	    /* if (len & 1) */
    beq	    L(sumN1)
    lhz	    R(10), 4(R(7))  /* w0 = match->weight */
    add	    R(6),R(10),R(6) /* weight_sum += w0 */
L(sumN1):

L(reduce):
    mr	    R(7),R(4)	    /* match = matchset   */
    mr	    R(9),R(4)	    /* tail = matchset   */
    divw    R(8),R(6),R(3)  /* mean_weight = weight_sum / len */ 
    li	    R(6),0	    /* weight_sum = 0 */
    blt	    cr1,L(reduceLt2)

    mtctr   R(15)	    /* ctr = len/2 */

L(reduceLoop):
    lwzu    R(12),4(R(7))   /* m0 = *(++match) */
    srwi    R(10),R(12),16  /* w0 = m0.weight  */
    lwzu    R(14),4(R(7))   /* m1 = *(++match) */
    srwi    R(11),R(14),16  /* w1 = m1.weight  */
    cmpw    cr0,R(10),R(8)  /* cmp w0 mean_weight */
    cmpw    cr1,R(11),R(8)  /* cmp w1 mean_weight */
    bgt	    cr0,L(skip0)
    stwu    R(12),4(R(9))   /* *(++tail) = m0 */
    add	    R(6),R(10),R(6) /* weight_sum += w0 */
L(skip0):
    bgt	    cr1,L(skip1)
    stwu    R(14),4(R(9))   /* *(++tail) = m1 */
    add	    R(6),R(11),R(6) /* weight_sum += w1 */
L(skip1):
    bdnz    L(reduceLoop)
L(reduceLt2):
    andi.   R(0),R(3),1	    /* if (len & 1) */
    beq	    L(reduceN1)
    lwzu    R(12),4(R(7))   /* m0 = *(++match) */
    srwi    R(10),R(12),16  /* w0 = m0.weight  */
    cmpw    cr0,R(10),R(8)  /* cmp w0 mean_weight */
    bgt	    cr0,L(skip3)
    stwu    R(12),4(R(9))   /* *(++tail) = m0 */
    add	    R(6),R(10),R(6) /* weight_sum += w0 */
L(skip3):
L(reduceN1):

    subic.  R(5),R(5),1	    /* reduction_times-- (set cr0) */
    subf    R(3),R(4),R(9)  /* len*4 = tail - matchset */
    srawi   R(3),R(3),2	    /* len = (len*4) >> 2 */
    cmpwi   cr1,R(3),2	    /* cmp len 2  */
    cror    10,2,4	    /* cr2[eq] = cr0[eq] | cr1[lt] */
    srawi   R(15),R(3),1    /* len/2 */
    bne	    cr2,L(reduce)

    stwu    R(3),0(R(4))    /* matchset->len = len */
    lwz     R(14),-4(R(1))  /* restore nonvolatile register r14 */
    lwz     R(15),-8(R(1))  /* restore nonvolatile register r15 */
#ifdef AMBER_ENABLE
    bl      amber_signal
    mtlr    R(16)
    lwz     R(16),-12(R(1)) /* restore nonvolatile register r16 */
#endif
    blr
#ifdef __linux__
L(end):
	.size	G(sub_mean_reduction_ppc),L(end)-G(sub_mean_reduction_ppc)
#endif
