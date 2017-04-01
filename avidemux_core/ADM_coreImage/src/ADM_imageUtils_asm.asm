;
;
%define private_prefix adm
%define public_prefix  adm

%include "admx86util.asm"

; cglobal functon_name, num_args, num_regs, num_xmm_regs
;static uint8_t tinyAverage(uint8_t *dst, uint8_t *src1, uint8_t *src2,uint32_t l)
;{
;
;uint8_t *s1,*s2,*d1;
;int a1,a2;
;        s1=src1;
;        s2=src2;
;        
;        d1=dst;
;          for(int y=0;y<l;y++)
;                {
;                        a1=*s1;
;                        a2=*s2;
;                        a1=a1+a2;
;                        a1>>=1;
;                        if(a1<0) a1=0;
;                        if(a1>255) a1=255;
;                        *d1=a1;                         
;
;                        s1++;
;                        s2++;
;                        d1++;
;                }
;        
;        return 1;
;}
;
SECTION .text
INIT_MMX mmx
cglobal tinyAverage, 4,4,3, dst, src, src2, len
    pxor            m2,m2
.loop
    movd           m0,[srcq]
    movd           m1,[src2q]
    punpcklbw      m0,m2 
    punpcklbw      m1,m2 
    paddw          m0,m1
    psrlw          m0,1
    packuswb       m0,m0 
    movd           [dstq],m0
    sub            lenq,1
    jz             .ret
    add            srcq,4
    add            src2q,4
   add             dstq,1
    jl             .loop
.ret
    REP_RET

