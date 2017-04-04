;
;
%define private_prefix adm
%define public_prefix  adm

%include "admx86util.asm"
;
;
;
;static inline void YUV444_chroma_MMX(uint8_t *src,uint8_t *dst,uint8_t *dst2,int w,int h,int s,int s2)
;{
;    int step=w/4;
;    int left=w-4*step;
;    uint8_t *xsrc=src;
;    uint8_t *xdst=dst;
;    uint8_t *xdst2=dst2;
;    for(int y=0;y<h;y++)
;    {
;        xsrc=src;
;        xdst=dst;
;        xdst2=dst2;
;        for(int x=0;x<step;x++)
;        {
;                        __asm__ volatile(
;                        "movq           (%0),%%mm0 \n"
;                        "movq           8(%0),%%mm1 \n"
;                        "movq           16(%0),%%mm2 \n"
;                        "movq           24(%0),%%mm3 \n"
;        
;                        "movq           %%mm0,%%mm4\n"
;                        "movq           %%mm1,%%mm5\n"
;                        "movq           %%mm2,%%mm6\n"
;                        "movq           %%mm3,%%mm7\n"
;
;                        "punpcklbw       %%mm2,%%mm0\n"
;                        "punpcklbw       %%mm3,%%mm1\n"
;                        "punpcklbw       %%mm1,%%mm0\n"
;                       
;                        "movd           %%mm0,(%1) \n"                       
;
;                        "psrlw          $8,%%mm4\n"
;                        "psrlw          $8,%%mm5\n"
;                        "psrlw          $8,%%mm6\n"
;                        "psrlw          $8,%%mm7\n"
;
;                        "punpcklbw       %%mm6,%%mm4\n"
;                        "punpcklbw       %%mm7,%%mm5\n"
;                        "punpcklbw       %%mm5,%%mm4\n"
;
;
;                        "movd           %%mm4,(%2) \n"                       
;                        :: "r"(xsrc),"r"(xdst),"r"(xdst2)
;                        :"memory"
;                        );
;                        xsrc+=32;
;                        xdst+=4;
;                        xdst2+=4;
;            }
;        for(int i=0;i<left;i++)
;        {
;             xdst[i]=xsrc[8*i];
;             xdst2[i]=xsrc[8*i+1];
;        }
;        dst+=s;
;        dst2+=s2;
;        src+=4*w*4;
;    }
section .text
INIT_MMX mmx
cglobal YUV444_chroma, 4,4,8, src, dst, dst2, w4
.again
        movq           m0,[srcq]
        movq           m1,8[srcq]
        movq           m2,16[srcq]
        movq           m3,24[srcq]

        movq           m4,m0
        movq           m5,m1
        movq           m6,m2
        movq           m7,m3

        punpcklbw      m0,m2
        punpcklbw      m1,m3
        punpcklbw      m0,m1
        movd           [dstq],m0
        psrlw          m4,8
        psrlw          m5,8
        psrlw          m6,8
        psrlw          m7,8

        punpcklbw      m4, m6
        punpcklbw      m5, m7
        punpcklbw      m4, m5

        movd           [dst2q],m4

        add            srcq,32
        add            dstq,4
        add            dst2q,4
        sub            w4d,1
        jnz             .again
        REP_RET
;
;
;
INIT_MMX mmx
cglobal nv12_to_u_v_one_line,4,4,4, w8, dstu, dstv, src

.nv12_again   
       movq           m0 , [srcq]
       movq           m1 , 8[srcq]
       movq           m2, m0
       movq           m3, m1

       psllw          m0,8
       psrlw          m0,8

       psllw          m1,8
       psrlw          m1,8


       packuswb       m0,m1

       psrlw          m2,8
       psrlw          m3,8

       packuswb       m2,m3

       movq           [dstvq],m0
       movq           [dstuq],m2

       add            srcq,16
       add            dstuq,8
       add            dstvq,8
       sub            w8q,1
       jnz            .nv12_again
       REP_RET


;eof
