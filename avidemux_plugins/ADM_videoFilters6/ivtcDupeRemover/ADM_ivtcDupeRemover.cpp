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

#define PERIOD 4
#define aprintf printf
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
                uint32_t        delta[PERIOD]; // List of image difference
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
        
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   ivtcDupeRemover,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_INTERLACING,            // Category
                        "ivtcRemover",            // internal name (must be uniq!)
                        "Remove IVTC dupe.",            // Display name
                        "Remove the duplicate frames present after ivtc." // Description
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
        configuration.show=true;
    }
    myName="ivtcDupeRemover";
    
    incomingNum=0;
    currentNum=0;
    phaseStart=0;
    dupeOffset=0;
    state=dupeSyncing;
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
        return ADMImage::lumaDiff(left,right,threshold);
    // Else we take line every 5 or 9 lines
    
    int scale=1+((int)configuration.mode*4);
    
    ADMImageRef refLeft(left->GetWidth(PLANAR_Y),left->GetHeight(PLANAR_Y)/scale);
    ADMImageRef refRight(right->GetWidth(PLANAR_Y),right->GetHeight(PLANAR_Y)/scale);
    
    refLeft._planes[0]=left->_planes[0];
    refLeft._planeStride[0]=left->_planeStride[0]/scale;
    
    refRight._planes[0]=right->_planes[0];
    refRight._planeStride[0]=right->_planeStride[0]/scale;
    
     return ADMImage::lumaDiff(&refLeft,&refRight,threshold);

    
}
/**
 * \fn lookupSync
 * \brief Try to search for a sequence
 * @return 
 */
ivtcDupeRemover::dupeState ivtcDupeRemover::searchSync()
{
    ADMImage *images[PERIOD+1];
    
    
    aprintf("Searching sync\n");
    
    for(int i=0;i<(PERIOD+1);i++)
    {
        images[i]=vidCache->getImage(incomingNum+i);
        if(!images[i])
        {
            vidCache->unlockAll();
            return dupeSyncing;
        }
    }
    
    int film=0;
    for(int i=0;i<PERIOD;i++)
    {
        delta[i]=0;
        if((images[i+1]->Pts-images[i]->Pts)> 41000) // 24 fps
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
    // Ok, they are all NTSC,  let's find the minimum one
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
                sprintf(s,"%u",delta[i]);
                out->printString(2,4+i,s);
            }
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
        diaElemUInteger   threshold(PX(threshold),QT_TR_NOOP("_Noise:"),0,255);
        diaElemToggle     show(PX(show),QT_TR_NOOP("_Show:"));

        
        diaMenuEntry menuMode[]={
                {0,      QT_TRANSLATE_NOOP("adm","Full")},
                {1,      QT_TRANSLATE_NOOP("adm","Fast")},
                {2,      QT_TRANSLATE_NOOP("adm","VeryFast")}
                };

        
        
        diaElemMenu      eMode(&configuration.mode,QT_TRANSLATE_NOOP("adm","_Frame rate change:"),3,menuMode);

        diaElem *elems[3]={&threshold,&show,&eMode};
        diaFactoryRun(QT_TR_NOOP("DupeRemover"),3,elems);
        return true;
}


/************************************************/
//EOF
