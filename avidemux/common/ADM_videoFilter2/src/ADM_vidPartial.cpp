/***************************************************************************
  \name partialFilter
  \brief only perform another filter operation on a limited time range, else bypass it
  \author Mean 2016
 
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
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "partial.h"
#include "partial_desc.cpp"

extern ADM_coreVideoFilter *ADM_vf_createFromTag(uint32_t tag, ADM_coreVideoFilter *last, CONFcouple *couples);
extern uint32_t    ADM_vf_getTagFromInternalName(const char *name);

ADM_coreVideoFilter *createPartialFilter(const char *internalName,CONFcouple *couples);
/**
    \class partialFilter
*/
class partialFilter : public  ADM_coreVideoFilter
{
  public:
    class  trampolineFilter : public  ADM_coreVideoFilter
    {
    public:
                            trampolineFilter(ADM_coreVideoFilter *previous,CONFcouple *conf=NULL);
       virtual             ~trampolineFilter();

       virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
       virtual bool         getNextFrame(uint32_t *frameNumber,ADMImage *image);              /// Dont mix getFrame & getNextFrame !     
       virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
       virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
       virtual void         setCoupledConf(CONFcouple *couples);
       virtual bool         configure(void) ;                          /// Start graphical user interface
       virtual uint64_t     getAbsoluteStartTime(void)   ;              /// Return the absolute offset of the current frame. Used to display time of for filter
       virtual bool         goToTime(uint64_t usSeek) {return true;}   /// Start graphical user interface
    };
  
  
protected:
                trampolineFilter *trampoline;
                ADM_coreVideoFilter *sonFilter;
                ADMImage    *myImage;
                partial      configuration;
                bool         byPass;
                char         description[2048];
                
                
                bool        isInRange(uint64_t tme);
                
public:
                            partialFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~partialFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getNextFrameForSon(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
		virtual void         setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void);   
        virtual bool         goToTime(uint64_t usSeek);   /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   partialFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_ALL,         // UI
                        VF_TRANSFORM,            // Category
                        "partial",            // internal name (must be uniq!)
                        "Make a filter partial.",            // Display name
                        "Limit the actions of some filters to a time range." // Description
                    );

// Now implements the interesting parts
/**
    \fn partialFilter
    \brief constructor
*/
partialFilter::partialFilter(  ADM_coreVideoFilter *in,CONFcouple *setup) : ADM_coreVideoFilter(in,setup)
{
    myImage=NULL;
    trampoline=NULL;
    sonFilter=NULL;
    byPass=true;
    
    // Step 1 : Load configuration
    if(!setup || !ADM_paramLoad(setup,partial_param,&configuration))
    {
        ADM_warning("Oops");
    }
    // Ok, create trampoline & son
    trampoline=new trampolineFilter(this,NULL);
    // Create swallowed filter
    // Get tag from name
    uint32_t tag;
    // get tag from name
    tag=ADM_vf_getTagFromInternalName(configuration.filterName.c_str());
    // spawn 
    int nbSonParam=setup->getSize()-3;
    ADM_assert(nbSonParam>=0);
    
    CONFcouple newParams(nbSonParam);
    for(int i=0;i<nbSonParam;i++)
    {
      char *key,*val;
      setup->getInternalName (i,&key,&val);
      newParams.setInternalName (key,val);
    }
    
    sonFilter=ADM_vf_createFromTag(tag, trampoline, &newParams);
    ADM_assert(sonFilter);
    // son = new filer(trampoline)
    // TODO FIXME
    if(!configuration.startBlack)
      byPass=false;
    return;
}
/**
    \fn partialFilter
    \brief destructor
*/
partialFilter::~partialFilter()
{
    if(myImage) delete myImage;
    myImage=NULL;
    if(sonFilter) delete sonFilter;
    sonFilter=NULL;
    if(trampoline) delete trampoline;
    trampoline=NULL;
}

/**
 * \fn getNextFrameForSon
 */
bool         partialFilter::getNextFrameForSon(uint32_t *fn,ADMImage *image)
{
  return previousFilter->getNextFrame(fn,image);
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool partialFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    if(byPass)
    {
        //
        if(false==previousFilter->getNextFrame(fn,image))
        {
            ADM_warning("partial : Cannot get frame\n");
            return false;
        }
        if(!isInRange(image->Pts))
        {
            byPass=true;
        }
        return true;
    }
    // Switch to the son instead
    if(false==sonFilter->getNextFrame(fn,image))
    {
         ADM_warning("partial : Cannot get frame from partial filter\n");
         return false;
    }
    if(image->Pts<configuration.startBlack || image->Pts>configuration.endBlack)
      byPass=false;
    return true;
}

/**
 */
bool partialFilter::isInRange(uint64_t tme)
{
   if(tme<configuration.startBlack || tme>configuration.endBlack)
     return false;
   return true;
}
/**
 */
bool         partialFilter::goToTime(uint64_t usSeek)
{
  bool r=previousFilter->goToTime(usSeek);
  byPass=!isInRange(usSeek);
  return r;
}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         partialFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, partial_param,&configuration);
}

void partialFilter::setCoupledConf(CONFcouple *couples)
{
    ADM_paramLoad(couples, partial_param, &configuration);
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *partialFilter::getConfiguration(void)
{
  sprintf(description,"Partial : %s -- ",ADM_us2plain(configuration.startBlack*1000));
  strcat(description,ADM_us2plain(configuration.endBlack*1000));
  strcat(description,sonFilter->getConfiguration());
  return description;
}

/**
    \fn configure
*/
bool partialFilter::configure( void)
{
	   return true;
}

/**
 */
 partialFilter::trampolineFilter::trampolineFilter(ADM_coreVideoFilter *previous,CONFcouple *conf) : ADM_coreVideoFilter(previous,conf)
 {
   
 }
 partialFilter::trampolineFilter::~trampolineFilter()
{
  
}
const char   * partialFilter::trampolineFilter::getConfiguration(void)
{
  return ""; // normally never used
}

FilterInfo  * partialFilter::trampolineFilter::getInfo(void)
{
  return previousFilter->getInfo();
}
bool          partialFilter::trampolineFilter::getCoupledConf(CONFcouple **couples) 
{
  return false; // never called
}
void          partialFilter::trampolineFilter::setCoupledConf(CONFcouple *couples)
{
    // never called
}
bool          partialFilter::trampolineFilter::configure()
{
    return true; // never called
}
uint64_t          partialFilter::trampolineFilter::getAbsoluteStartTime()
{
    return previousFilter->getAbsoluteStartTime(); // never called
}

bool         partialFilter::trampolineFilter::getNextFrame(uint32_t *frameNumber,ADMImage *image)
{
  partialFilter *p=(partialFilter *)previousFilter;
  return p->getNextFrameForSon(frameNumber,image);
}
/************************************************/

/**
 * \fn createPartialFilter
 * @param internalName
 * @param couples
 * @return 
 */
ADM_coreVideoFilter *createPartialFilter(const char *internalName,CONFcouple *couples,ADM_coreVideoFilter *source)
{
  int sonNbItems=couples->getSize();
  CONFcouple tmp(3+sonNbItems);
  
  uint32_t start=1000;
  uint32_t end=5000;
  
  tmp.writeAsString("filterName",internalName);
  tmp.writeAsUint32 ("startBlack",start);
  tmp.writeAsUint32 ("endBlack",end);
  for(int i=0;i<sonNbItems;i++)
  {
      char *key,*val;
      couples->getInternalName (i,&key,&val);
      tmp.setInternalName (key,val);
  }
  ADM_coreVideoFilter *p=new partialFilter(source,&tmp);
  return p;  
}

namespace admPartial
{
    ADM_coreVideoFilter *create(ADM_coreVideoFilter *previous,CONFcouple *conf)
    {
      return new partialFilter(previous,conf);
    }
    void destroy(ADM_coreVideoFilter *c)
    {
      partialFilter *p=(partialFilter *)c;
      delete p;
      p=NULL;
    }
    int supportedUI()
    {
      return ADM_UI_ALL;
    }
    int               neededFeatures(void) {return 0;}
    uint32_t          apiVersion() 
    {
      return VF_API_VERSION;
    }
    bool              getPluginVersion(uint32_t *major, uint32_t *minor, uint32_t *patch)
    {
        *major=1;
        *minor=0;
        *patch=0;
        return true;
    }
    const char       *getString()
    {
      return "partial";
    }
    VF_CATEGORY       getCategory(void)
    {
      return VF_HIDDEN;
    }
    bool              partializable(void){return false;}    
    
}; // namespace

class fakePartialPlugin : public ADM_vf_plugin
{
public:  
  fakePartialPlugin() 
  {
        create=admPartial::create;
        destroy=admPartial::destroy;
        supportedUI=admPartial::supportedUI;
        neededFeatures=admPartial::neededFeatures;
        getApiVersion=admPartial::apiVersion;
        getFilterVersion=admPartial::getPluginVersion;
        getDesc=admPartial::getString;
        getInternalName=admPartial::getString;
        getDisplayName=admPartial::getString;
        getCategory=admPartial::getCategory;
        partializable=admPartial::partializable;

        nameOfLibrary="";
        tag=VF_PARTIAL_FILTER;
        
        info.internalName="partial";
        info.displayName="partial";
        info.desc="partial";
        info.category=VF_HIDDEN;        
  }
  
};

static fakePartialPlugin fakePlugin;
ADM_vf_plugin *getFakePartialPlugin()
{
  return &fakePlugin;
}

//EOF
