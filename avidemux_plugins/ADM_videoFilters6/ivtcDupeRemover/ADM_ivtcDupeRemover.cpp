/***************************************************************************

		Put a logon on video

    copyright            : (C) 2007 by mean
    email                : fixounet@free.fr
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_default.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_vidMisc.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "ivtcDupeRemover.h"
#include "ivtcDupeRemover_desc.cpp"

#if defined( ADM_CPU_X86) && !defined(_MSC_VER)
        #define CAN_DO_INLINE_X86_ASM
#endif

#define PERIOD 4
#if 0
#define aprintf printf
#else
#define aprintf(...) {}
#endif


#define MARK_PROGRESSIVE 'PRGS'
#define MARK_DUPLICATE   'DUPE'

/**
    \class ivtcDupeRemover
*/
class ivtcDupeRemover : public  ADM_coreVideoFilterCached
{
public:
                enum dupeState
                {
                    dupeSyncing,
                    dupeSynced,
                    dupePassThrough
                };
                enum searchMode
                {
                    full=0,
                    fast=1,
                    veryFast=2
                };
                int             incomingNum; // Incoming frame number
                int             currentNum;  // outgoing frame number
                int             phaseStart;  // Frame Number of 1st frame of cycle
                uint64_t        phaseStartPts; // its PTS
                int             dupeOffset;  // offset of the duplicate to drop,  i.e. abs number = phaseStart+dupeOffset
                dupeState       state;       // Current finite state machine
                uint32_t        delta[PERIOD+1]; // List of image difference
                unsigned int    hints[PERIOD+1]; // From D. Graft ivtc if used
protected:
                dupeRemover        configuration;
                dupeState          searchSync();
                bool               postProcess(ADMImage *in,ADMImage *out,uint64_t pts);
                uint32_t           computeDelta(ADMImage *left,ADMImage *right, int threshold);
public:
                                   ivtcDupeRemover(ADM_coreVideoFilter *previous,CONFcouple *conf);
                                   ~ivtcDupeRemover();
                bool               goToTime(uint64_t usSeek);
        virtual const char         *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool               getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool               getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
        virtual void               setCoupledConf(CONFcouple *couples);
        virtual bool               configure(void);                           /// Start graphical user interface
                uint32_t           lumaDiff(ADMImage *src1,ADMImage *src2,uint32_t noise);

};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ivtcDupeRemover,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "ivtcRemover",            // internal name (must be uniq!)
                        QT_TRANSLATE_NOOP("ivtcRemover","Remove IVTC dupe."),            // Display name
                        QT_TRANSLATE_NOOP("ivtcRemover","Remove the duplicate frames present after ivtc.") // Description
                    );

// Now implements the interesting parts
/**
    \fn ivtcDupeRemover
    \brief constructor
*/
ivtcDupeRemover::ivtcDupeRemover(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilterCached(11,in,setup)
{
    if(!setup || !ADM_paramLoad(setup,dupeRemover_param,&configuration))
    {
        // Default value
        configuration.threshold=5;
        configuration.show=false;
        configuration.mode=1; // fast!

    }
    myName="ivtcDupeRemover";

    incomingNum=0;
    currentNum=0;
    phaseStart=0;
    dupeOffset=0;
    state=dupeSyncing;
    // ajust fps
    double fps=info.frameIncrement;
    
    fps*=PERIOD+1;
    fps/=PERIOD;
    info.frameIncrement=fps;
}
/**
    \fn ivtcDupeRemover
    \brief destructor
*/
ivtcDupeRemover::~ivtcDupeRemover()
{
}
/**
 * \fn computeDelta
 * @param left
 * @param right
 * @param threshold
 * @return
 */
uint32_t ivtcDupeRemover::computeDelta(ADMImage *left,ADMImage *right, int threshold)
{
    if(!configuration.mode)
        return lumaDiff(left,right,threshold);
    // Else we take line every 5 or 9 lines

    int scale=1+((int)configuration.mode*4);

    ADMImageRef refLeft(left->GetWidth(PLANAR_Y),left->GetHeight(PLANAR_Y)/scale);
    ADMImageRef refRight(right->GetWidth(PLANAR_Y),right->GetHeight(PLANAR_Y)/scale);

    refLeft._planes[0]=left->_planes[0];
    refLeft._planeStride[0]=left->_planeStride[0]/scale;

    refRight._planes[0]=right->_planes[0];
    refRight._planeStride[0]=right->_planeStride[0]/scale;

     return lumaDiff(&refLeft,&refRight,threshold);


}
/**
 * \fn lookupSync
 * \brief Try to search for a sequence
 * @return
 */
ivtcDupeRemover::dupeState ivtcDupeRemover::searchSync()
{
    ADMImage *images[PERIOD+1];


    aprintf("dupeRemover : Searching sync\n");

    for(int i=0;i<(PERIOD+1);i++)
    {
        images[i]=vidCache->getImage(incomingNum+i);
        if(!images[i])
        {
            vidCache->unlockAll();
            aprintf("No image (%d)\n",i);
            return dupeSyncing;
        }
        if(GetHintingData(images[i]->GetReadPtr(PLANAR_Y),hints+i)) // Returns true on error
        {
            aprintf("[%d] No hint\n",i);
            hints[i]=0;
        }else
        {
            aprintf("[%d] Got hint %x\n",i,hints[i]);
        }
    }

    int film=0;
    for(int i=0;i<PERIOD;i++)
    {
        int deltaPts=(int)(images[i+1]->Pts-images[i]->Pts);
        delta[i]=0;
        aprintf("DeltaPts=%d\n",deltaPts);
        if(deltaPts> 41000) // 24 fps
            film++;
    }
    // All of it is 24 fps, pass through
    if(film==PERIOD)
    {
        aprintf("It is all film\n");
        vidCache->unlockAll();
        return dupePassThrough;
    }
    // It is mixed, keep syncing
    if(film)
    {
        aprintf("It is mixed fps\n");
        vidCache->unlockAll();
        return dupeSyncing;
    }
    
    // Do we have hints ?
    int nbProgressiveHint=0;
    int nbDupeHint=0;
    bool found=false;
    for(int i=0;i<PERIOD+1;i++)
    {
        switch(hints[i])
        {
            case MARK_PROGRESSIVE: nbProgressiveHint++;break;
            case MARK_DUPLICATE:  nbDupeHint++;dupeOffset=i;break;
            default:break;
        }
    }
    if(nbProgressiveHint==PERIOD && nbDupeHint==1)
    {
        aprintf("Hinting is giving the dupe at %d\n",dupeOffset);
        found=true;
    }

    
    // Ok, they are all NTSC,  let's find the minimum one
    if(!found)
    {
        aprintf("Lets look for the closest one\n");
        for(int i=0;i<PERIOD;i++)
        {
            if(images[i] && images[i+1])
                delta[i]=computeDelta(images[i],images[i+1],configuration.threshold);
            else
                delta[i]=0x70000000; // Big number
        }
        uint32_t minDelta=0x7f000000;
        for(int i=0;i<PERIOD;i++)
        {
            if(minDelta>delta[i])
            {
                minDelta=delta[i];
                dupeOffset=i;

            }
        }
    }
    phaseStart=incomingNum;
    phaseStartPts=images[0]->Pts;
    vidCache->unlockAll();
    return dupeSynced;
}
/**
 * \fn dupeState2string
 * @return
 */
static const char *dupeState2string(ivtcDupeRemover::dupeState state)
{
      const char *m="";
      switch(state)
            {
                case    ivtcDupeRemover::dupeSyncing:     m="Syncing";break;
                case    ivtcDupeRemover::dupeSynced:      m="dupeSynced";break;
                case    ivtcDupeRemover::dupePassThrough: m="dupePassThrough";break;
                default: ADM_assert(0);break;
            }
      return m;
}
/**
 *
 * @param img
 * @return
 */
bool ivtcDupeRemover::postProcess(ADMImage *in,ADMImage *out,uint64_t newPts)
{
    if(in)
    {
        out->duplicateFull(in);
        if(newPts!=ADM_NO_PTS)
            out->Pts=newPts;
        if(configuration.show)
        {
            char s[256];
            const char *m=dupeState2string(state);
            out->printString(2,2,m);
            for(int i=0;i<PERIOD;i++)
            {
                sprintf(s,"Diff:%u",delta[i]);
                out->printString(2,4+i,s);
                sprintf(s,"Hint:%x",hints[i]);
                out->printString(2,11+i,s);
            }
            sprintf(s,"Hint:%x",hints[PERIOD]);
            out->printString(2,11+PERIOD,s);

        }
    }
    return true;
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
#define END_PROCESS(i,image,newPts,nextState) \
                *fn=currentNum; \
                currentNum++; \
                postProcess(i,image,newPts); \
                state=nextState; \
                vidCache->unlockAll(); \
                if(i) \
                    return true; \
                return false;


bool ivtcDupeRemover::getNextFrame(uint32_t *fn,ADMImage *image)
{
    aprintf("Current state is %s\n",dupeState2string(state));
    switch(state)
    {
        case dupeSynced:
            {
              ivtcDupeRemover::dupeState nextState=dupeSynced;
              int count=incomingNum-phaseStart;
              if(count>dupeOffset)
                  count--;
              if((incomingNum-phaseStart)==dupeOffset)
              {
                  aprintf("This is the dupe (%d), skipping\n",incomingNum);
                  incomingNum++;
              }
              aprintf("Cound in sequence is %d\n",count);
              ADMImage *i=vidCache->getImage(incomingNum);
              incomingNum++;
              if((incomingNum-phaseStart)>PERIOD)
              {
                  nextState=dupeSyncing;
              }
              uint64_t newPts=phaseStartPts+count*41666;
              END_PROCESS(i,image,newPts,nextState);
              break;
            }
            break;
        case dupePassThrough:
            {
              ADMImage *i=vidCache->getImage(incomingNum);
              incomingNum++;
              if((incomingNum-phaseStart)>(PERIOD))
                      state=dupeSyncing;
             END_PROCESS(i,image,ADM_NO_PTS,dupePassThrough);
             break;
            }
        case dupeSyncing:
            {
                ivtcDupeRemover::dupeState nextState=searchSync();
                // 1st one gets through
                aprintf(">>State = %s, in =%d, out=%d\n",dupeState2string(nextState),incomingNum,currentNum);
                ADMImage *i=vidCache->getImage(incomingNum);
                if(i)
                    aprintf(">>PTS=%s\n",ADM_us2plain(i->Pts));
                incomingNum++;
                END_PROCESS(i,image,ADM_NO_PTS,nextState);
                break;
            }
        default:
              ADM_assert(0);

              break;

      }
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         ivtcDupeRemover::getCoupledConf(CONFcouple **couples)
{

    return ADM_paramSave(couples, dupeRemover_param,&configuration);
}

void ivtcDupeRemover::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, dupeRemover_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *ivtcDupeRemover::getConfiguration(void)
{
    static char bfer[1024];

    sprintf(bfer,"IVTC Dupe Removed : Threshold =%u",(unsigned int)configuration.threshold);
    return bfer;
}
/**
 *
 * @param usSeek
 * @return
 */
bool         ivtcDupeRemover::goToTime(uint64_t usSeek)
{
    vidCache->flush();
    state=dupeSyncing;
    incomingNum=0;
    currentNum=0;
    return previousFilter->goToTime(usSeek);
}
/**
    \fn configure
*/
bool ivtcDupeRemover::configure( void)
{

#define PX(x) &(configuration.x)
        diaElemUInteger   threshold(PX(threshold),QT_TRANSLATE_NOOP("ivtcRemover","_Noise:"),0,255);
        diaElemToggle     show(PX(show),QT_TRANSLATE_NOOP("ivtcRemover","_Show:"));

        diaMenuEntry menuMode[]={
                {0,      QT_TRANSLATE_NOOP("ivtcRemover","Full"), NULL},
                {1,      QT_TRANSLATE_NOOP("ivtcRemover","Fast"), NULL},
                {2,      QT_TRANSLATE_NOOP("ivtcRemover","VeryFast"), NULL}
        };

        diaElemMenu      eMode(&configuration.mode,QT_TRANSLATE_NOOP("ivtcRemover","_Frame rate change:"),3,menuMode);

        diaElem *elems[3]={&threshold,&show,&eMode};
        return diaFactoryRun(QT_TRANSLATE_NOOP("ivtcRemover","DupeRemover"),3,elems);
}
#ifdef CAN_DO_INLINE_X86_ASM
static uint64_t __attribute__((used)) FUNNY_MANGLE(noise64);
/**
*/
static uint32_t smallDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise, int count)
{
uint32_t df=0;
uint32_t delta;
    for(int x=0;x<count;x++)
    {
            delta=abs((int)(s1[x])-(int)(s2[x]));
            if(delta>noise)
                    df+=delta;
    }
    return df;
}

/**
 * \fn computeDiffMMX
 * @param s1
 * @param s2
 * @param noise
 * @param w
 * @param l
 * @param pitch1
 * @param pitch2
 * @return
 */
static uint32_t computeDiffMMX(uint8_t  *s1,uint8_t *s2,uint32_t noise,uint32_t w,uint32_t l, uint32_t pitch1, uint32_t pitch2)
{
uint32_t mod4,leftOver;
uint64_t noise2=(uint64_t )noise;

uint32_t result=0,tmpResult;
        noise64=noise2+(noise2<<16)+(noise2<<32)+(noise2<<48);

        leftOver=w&7;

         __asm__ volatile(
                         "pxor %%mm7,%%mm7\n"
                         "movq " Mangle(noise64)", %%mm6\n"
                :::  "memory"
                 );

          for(int y=0;y<l;y++)
          {
                mod4=w>>3;
                if(leftOver)
                    result+=smallDiff(s1+mod4*8,s2+mod4*8,noise,leftOver);
                uint8_t *tmpS1=s1;
                uint8_t *tmpS2=s2;

                __asm__ volatile(
                        "pxor           %%mm3,%%mm3\n"
                        "1:"
                // LEFT
                        "movd           (%0),  %%mm0 \n"
                        "movd           (%1),  %%mm1 \n"
                        "punpcklbw      %%mm7, %%mm0 \n"
                        "punpcklbw      %%mm7, %%mm1 \n"

                        "movq           %%mm0, %%mm2 \n"
                        "psubusw        %%mm1, %%mm2 \n"
                        "psubusw        %%mm0, %%mm1 \n"
                        "por            %%mm1, %%mm2 \n" // SAD

                        "movq           %%mm2, %%mm0 \n"
                        "pcmpgtw        %%mm6, %%mm2 \n" // Threshold against noise
                        "pand           %%mm2, %%mm0 \n" //
                        "movq           %%mm0, %%mm5 \n" //  %mm5 is the  A1 A2 A3 A4, we want the sum later
                // RIGHT
                        "movd           4(%0),  %%mm0 \n"
                        "movd           4(%1),  %%mm1 \n"
                        "punpcklbw      %%mm7, %%mm0 \n"
                        "punpcklbw      %%mm7, %%mm1 \n"

                        "movq           %%mm0, %%mm2 \n"
                        "psubusw        %%mm1, %%mm2 \n"
                        "psubusw        %%mm0, %%mm1 \n"
                        "por            %%mm1, %%mm2 \n" // SAD

                        "movq           %%mm2, %%mm0 \n"
                        "pcmpgtw        %%mm6, %%mm2 \n" // Threshold against noise
                        "pand           %%mm2, %%mm0 \n" // mm0 is B1 B2 B3 B4

                        "paddW          %%mm5, %%mm0 \n"

                // PACK
                        "movq           %%mm0, %%mm1 \n" // MM0 is a b c d and we want
                        "psrlq          $16,  %%mm1 \n"  // mm3+=a+b+c+d

                        "movq           %%mm0, %%mm2 \n"
                        "psrlq          $32,  %%mm2 \n"

                        "movq           %%mm0, %%mm4 \n"
                        "psrlq          $48,  %%mm4 \n"

                        "paddw          %%mm1, %%mm0 \n"
                        "paddw          %%mm2, %%mm4 \n"
                        "paddw          %%mm4, %%mm0 \n" // MM0 is the sum

                        "psllq          $48,  %%mm0 \n"
                        "psrlq          $48,  %%mm0 \n" // Only keep 16 bits

                        "paddw          %%mm0, %%mm3 \n" /* PADDQ is SSE2 */
                        "add            $8,%0      \n"
                        "add            $8,%1      \n"
                        "sub            $1,%2      \n"
                        "jnz            1b         \n"

                : "=r" (tmpS1),"=r" (tmpS2),"=r"(mod4)
                : "0"(tmpS1),"1"(tmpS2),"2"(mod4)
                : "memory","0","1","2"
                );
                __asm__ volatile(

                        "movd           %%mm3,(%0)\n"
                        "emms\n"
                :: "r"(&tmpResult)
                );
                result+=tmpResult;
                s1+=pitch1;
                s2+=pitch2;
         }
        // Pack result
        return result;
}


#endif
/**

*/
/* 3000 * 3000 max size, using uint32_t is safe... */
static uint32_t computeDiff(uint8_t  *s1,uint8_t *s2,uint32_t noise,int w,int h, int stride1, int stride2)
{
uint32_t df=0;
uint32_t delta;

    for(int y=0;y<h;y++)
    {
        for(int x=0;x<w;x++)
        {
                delta=abs((int)(s1[x])-(int)(s2[x]));
                if(delta>noise)
                        df+=delta;
        }
        s1+=stride1;
        s2+=stride2;
    }
    return df;
}
/**
*/
uint32_t ivtcDupeRemover::lumaDiff(ADMImage *src1,ADMImage *src2,uint32_t noise)
{

#ifdef CAN_DO_INLINE_X86_ASM
uint32_t r1,r2;
        if(CpuCaps::hasMMX())
        {
                uint32_t a= computeDiffMMX(YPLANE(src1),YPLANE(src2),noise,
                    src1->GetWidth(PLANAR_Y),src1->GetHeight(PLANAR_Y),
                    src1->GetPitch(PLANAR_Y),src2->GetPitch(PLANAR_Y));
#if 0
                uint32_t b= computeDiff(YPLANE(src1),YPLANE(src2),noise,
                    src1->GetWidth(PLANAR_Y),src1->GetHeight(PLANAR_Y),
                    src1->GetPitch(PLANAR_Y),src2->GetPitch(PLANAR_Y));
                printf("MMX = %u, native =%u\n",a,b);
#endif
                return a;

        }
#endif
        return computeDiff(YPLANE(src1),YPLANE(src2),noise,
                src1->GetWidth(PLANAR_Y),src1->GetHeight(PLANAR_Y),
                src1->GetPitch(PLANAR_Y),src2->GetPitch(PLANAR_Y));
}

/************************************************/
//EOF
