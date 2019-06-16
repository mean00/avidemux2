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
        RET
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
       RET

;yuv444_MMX
;  xdst=dst;
;        for(int x=0;x<step;x++)
;        {
;                        __asm__ volatile(
;                        "movq           (%0),%%mm0 \n"
;                        "pand           %%mm7,%%mm0\n"
;                        "movq           8(%0),%%mm1 \n"
;
;                        "movq           16(%0),%%mm2 \n"
;                        "pand           %%mm7,%%mm1\n"
;                        "pand           %%mm7,%%mm2\n"
;                        "movq           24(%0),%%mm3 \n"
;                        "pand           %%mm7,%%mm3\n"
;
;                        "packuswb       %%mm1,%%mm0\n"
;                        "packuswb       %%mm3,%%mm2\n"
;                        "psrlw          $8,%%mm0\n"
;                        "psrlw          $8,%%mm2\n"
;                        "packuswb       %%mm2,%%mm0\n"
;
;                        "movq           %%mm0,(%1) \n"
;
;
;                        :: "r"(xsrc),"r"(xdst)
;                        :"memory"
;                        );
;                        xsrc+=32;
;                        xdst+=8;
;            }

INIT_MMX mmx
cglobal YUV444Luma,4,4,5, w8, dst, src,mangle
           movq            m4,[mangleq]
.again3
            movq           m0,[srcq]
            pand           m0,m4
            movq           m1,8[srcq]
            movq           m2,16[srcq]
            pand           m1,m4
            pand           m2,m4
            movq           m3,24[srcq]
            pand           m3,m4
            packuswb       m0,m1
            packuswb       m2,m3
            psrlw          m0,8
            psrlw          m2,8
            packuswb       m0,m2
            movq           [dstq],m0
            add            srcq,32
            add            dstq,8
            sub            w8q,1
            jnz            .again3
            RET


;eof
;__asm__ volatile(
; "mov            %4,%3      \n" // local copy
;  "1:\n"
;  "movq           (%1),%%mm0   \n" // U
;  "movq           (%2),%%mm1   \n" // V
;  "movq           %%mm0,%%mm2  \n"
;  "movq           %%mm1,%%mm3  \n"
;
;  "punpcklbw      %%mm1,%%mm0  \n"
;  "punpckhbw      %%mm3,%%mm2  \n"
;  "movq           %%mm0,(%0)   \n"
;  "movq           %%mm2,8(%0)  \n"
;
;  "add            $16,%0       \n"
;  "add            $8,%1        \n"
;  "add            $8,%2        \n"
;  "sub            $1,%3        \n"
;  "jnz            1b           \n"
;
;  :
;  : "r"(ddst),"r"(u),"r"(v),"r"(x),"r"(mod8)
;  : "memory"
;  );
cglobal uv_to_nv12,4,4,4, u, v, dst,w8

.nv12
  movq           m0,[uq]
  movq           m1,[vq]
  movq           m2,m0
  movq           m3,m1

  punpcklbw      m0,m1
  punpckhbw      m2,m3
  movq           [dstq],m0
  movq           8[dstq],m2

  add            dstq,16
  add            uq,8
  add            vq,8
  sub            w8q,1
  jnz            .nv12
  RET
