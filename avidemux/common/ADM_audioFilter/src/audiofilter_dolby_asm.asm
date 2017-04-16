;
;
%define private_prefix adm
%define public_prefix  adm

%include "admx86util.asm"
;  : "r" (src1),"r" (src2),"r"(sum16),"r"(mod16)
;  __asm__(
;                        "xorps          %%xmm2,%%xmm2     \n" // carry
;                        "1: \n"
;                        "movaps         (%0),%%xmm0  \n" // src1
;                        "movaps         (%1),%%xmm1  \n" // src2
;                        "mulps          %%xmm1,%%xmm0 \n" // src1*src2
;                        "addps          %%xmm0,%%xmm2 \n" // sum+=src1*src2
;                        "add           $16,%0      \n"
;                        "add           $16,%1      \n"
;                        "sub           $1,%3      \n"
;                        "jnz             1b        \n"
;                        "movaps        %%xmm2,(%2)        \n"
;
;                : : "r" (src1),"r" (src2),"r"(sum16),"r"(mod16)
;                );
   
section .text
INIT_XMM sse2 ; SSE2 ?
cglobal dolby_asm, 4,4,4, src1, src2, sum, l
        xorps            m2,m2, m2
.again:
        movaps           m0,   [src1q]
        movaps           m1,   [src2q]
        mulps            m0,   m1
        addps            m2,   m0
        add              src1q,16
        add              src2q,16
        sub              ld,   1
        jnz             .again
        movaps          [sumq],m2
        RET

