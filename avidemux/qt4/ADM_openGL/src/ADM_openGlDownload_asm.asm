;
;
%define private_prefix adm
%define public_prefix  adm

%include "admx86util.asm"
;
;
;#ifdef CAN_DO_INLINE_X86_ASM
;static inline void glYUV444_MMXInit(void)
;{
;   static uint64_t __attribute__((used)) FUNNY_MANGLE(mask) = 0x00ff000000ff0000LL;
;
;    __asm__(" movq " Mangle(mask)", %%mm7\n" ::);
;}
;ADM_NO_OPTIMIZE static inline void glYUV444_MMX(const uint8_t *src2, uint8_t *dst2, const int width2)
;{
;    uint8_t *src=(uint8_t *)src2;
;    uint8_t *dst=(uint8_t *)dst2;
;    int width=width2;
;    int count=width/8;
;                    __asm__(
;                        "1:\n"
;                        "movq           (%0),%%mm0 \n"
;                        "pand           %%mm7,%%mm0\n"
;                        "movq           8(%0),%%mm1 \n"
;                        "pand           %%mm7,%%mm1\n";
;
;                        "movq           16(%0),%%mm2 \n"
;                        "pand           %%mm7,%%mm2\n"
;                        "movq           24(%0),%%mm3 \n"
;                        "pand           %%mm7,%%mm3\n"
;;
;                        "packuswb       %%mm1,%%mm0\n"
;                        "packuswb       %%mm3,%%mm2\n"
;                        "psrlw          $8,%%mm0\n"
;                       "psrlw          $8,%%mm2\n"
;                        "packuswb       %%mm2,%%mm0\n";;
;
;                        "movq           %%mm0,(%1)  \n"  
;                        "add            $32,%0      \n"
;                        "add            $8,%1       \n"
;                        "sub            $1,%2        \n"
;                        "jnz             1b         \n"
;                        
;                        : "=r"(src),"=r"(dst),"=r"(count)
;                        : "0"(src),"1"(dst),"2"(count)
;                        );
SECTION_RODATA
MASK:
      dq 0x00ff000000ff0000
section .text
INIT_MMX mmx
default rel
cglobal glYUV444_Init,0,0,0
        movq          m7,[MASK]
        RET

default abs
INIT_MMX mmx
cglobal glYUV444_luma, 3,4,4, src, dst, count
.luma:
   
        movq           m0,[srcq]
        pand           m0,m7
        movq           m1,8[srcq]
        pand           m1,m7


        movq           m2,16[srcq]
        pand           m2,m7
        movq           m3,24[srcq]
        pand           m3,m7

        packuswb       m0,m1
        packuswb       m2,m3
        psrlw          m0,8
        psrlw          m2,8
        packuswb       m0,m2

        movq           [dstq],m0
        add            srcq,32
        add            dstq,8
        sub            countd,1
        jnz             .luma
        RET
;"1:\n"
;                        "movq           (%0),%%mm0 \n"
;                        "pmov           %%mm0,%%mm4 \n"
;;                        "pand           %%mm7,%%mm0\n"
;                        "movq           8(%0),%%mm1 \n"
;                        "pmov           %%mm1,%%mm5 \n"
;;                        "pand           %%mm7,%%mm1\n"
;
;                        "movq           16(%0),%%mm2 \n"
;                        "pmov           %%mm2,%%mm6 \n"
;;                        "pand           %%mm7,%%mm2\n"
;                        "movq           24(%0),%%mm3 \n";
;                        "packuswb       %%mm1,%%mm0\n"
;                        "pmov           %%mm3,%%mm1 \n" // We have a copy in MM4/MM5/MM6/MM1
;                        "pand           %%mm7,%%mm3\n"
;
;                        // Pack luma
;                        "packuswb       %%mm3,%%mm2\n"
;                        "psrlw          $8,%%mm0\n"
;                        "psrlw          $8,%%mm2\n"
;                        "packuswb       %%mm2,%%mm0\n"
;                        "movq           %%mm0,(%1)  \n"  
;                            
;                        // now do chroma, it is similar    
;                            
;                        // Next..
;                        "add            $32,%0      \n"
;                        "add            $8,%1       \n"
;                        "sub            $1,%2        \n"
;                        "jnz             1b         \n"
;                        
;                        :  "=r"(src),"=r"(dstY),"=r"(dstU),"=r"(dstV),"=r"(count)

INIT_MMX mmx
cglobal glYUV444_luma2, 3,4,4, src, dstY, count
.chroma:
        movq           m0,[srcq]
        movq           m0,m4
        pand           m0,m7
        movq           m1,8[srcq]
        movq           m5,m1
        pand           m1,m7

        movq           m2,16[srcq]
        movq           m6,m2
        pand           m2,m7
        movq           m3,24[srcq]
        packuswb       m0,m1
        movq           m1,m3
        pand           m3,m7

        ; Pack luma
        packuswb       m2,m3
        psrlw          m0,8
        psrlw          m2,8
        packuswb       m0,m2
        movq           [dstYq],m0

        ; now do chroma, it is similar    

        ; Next..
        add            srcq,32
        add            dstYq,8
        sub            countd,1
        jnz            .chroma        
        RET                
                        

