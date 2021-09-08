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
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_videoFilterApi.h"
#include "DIA_factory.h"
#include "DIA_coreToolkit.h"
#include "ADM_vidMisc.h"
#include "partial.h"
#include "partial_desc.cpp"
#include "avi_vars.h"

#define DESC_MAX_LENGTH 2048

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
       virtual bool         getTimeRange(uint64_t *start, uint64_t *end); // Get the time a partialized filter is active
       virtual bool         goToTime(uint64_t usSeek); // needed for seekable preview when reconfiguring a partialized filter
    };
        friend class trampolineFilter;

protected:
                trampolineFilter *trampoline;
                ADM_coreVideoFilter *sonFilter;
                partial      configuration;
                bool         byPass;
                char         description[DESC_MAX_LENGTH];
                ADMImage     *intermediate;
                bool         hasIntermediate;
                uint32_t     intermediateFn;
                bool         sonFilterPreview;
                FilterInfo   previewFilterInfo;


                bool        isInRange(uint64_t tme);

public:
                            partialFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~partialFilter();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getNextFrameForSon(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual FilterInfo   *getInfo(void);
        virtual bool         getCoupledConf(CONFcouple **couples) ;     /// Return the current filter configuration
        virtual void         setCoupledConf(CONFcouple *couples);
        virtual bool         configure(void); /// Start graphical user interface
        virtual bool         getRange(uint64_t *start, uint64_t *end);
        virtual bool         goToTime(uint64_t usSeek);
        static  void         reconfigureCallback(void *cookie);
                void         reconfigureSon();
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
    trampoline=NULL;
    sonFilter=NULL;
    sonFilterPreview=false;
    byPass=true;

    intermediate=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    hasIntermediate=false;

    // Step 1 : Load configuration
    if(!setup)
        ADM_assert(0);
    // Only keep 3 parameters
    char *filterName=NULL;
    if(!setup->readAsString ("filterName",&filterName))
      {
        ADM_assert(0);
      }
    configuration.filterName=std::string(filterName);
    delete [] filterName;
    filterName=NULL;
    if(!setup->readAsUint32("startBlack",&(configuration.startBlack)))
      {
        ADM_assert(0);
      }
    if(!setup->readAsUint32("endBlack",&(configuration.endBlack)))
      {
        ADM_assert(0);
      }
    // sanity check
    if(configuration.endBlack < configuration.startBlack)
    {
        uint32_t swap=configuration.startBlack;
        configuration.startBlack=configuration.endBlack;
        configuration.endBlack=swap;
    }
    // Ok, create trampoline & son
    trampoline=new trampolineFilter(this,NULL);
    // Create swallowed filter
    // Get tag from name
    uint32_t tag;
    // get tag from name
    int nbSonParam=setup->getSize()-3;
    ADM_assert(nbSonParam>=0);

    ADM_info("Creating partial filter for %s, with %d params\n",configuration.filterName.c_str(),nbSonParam);
    tag=ADM_vf_getTagFromInternalName(configuration.filterName.c_str());
    // spawn

    CONFcouple newParams(nbSonParam);
    for(int i=0;i<nbSonParam;i++)
    {
      char *key,*val;
      setup->getInternalName (i+3,&key,&val);
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

    if(intermediate) delete intermediate;
    intermediate=NULL;
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
  if(sonFilterPreview)
      return previousFilter->getNextFrame(fn,image);
  if(!hasIntermediate)
    {
      ADM_warning("Partial filter requesting image, no image in store!!\n");
      return false;
    }

  *fn=intermediateFn;
  image->duplicateFull(intermediate);
  return true;
}
/**
    \fn getFrame
    \brief Get a processed frame
*/
bool partialFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{
    hasIntermediate=false;
    // 1 - Get image from previous filter
    if(false==previousFilter->getNextFrame(fn,image))
      {
        ADM_warning("partial : Cannot get frame\n");
        return false;
      }
    // 2 -Check if we are in range or not
    if(!isInRange(image->Pts))
    {
        return true; // we have the image already
    }
    // 3- we are in range, prepare for trampolining
    intermediate->duplicateFull(image);
    hasIntermediate=true;
    intermediateFn=*fn;

    // Switch to the son instead
    if(false==sonFilter->getNextFrame(&intermediateFn,image))
    {
         ADM_warning("partial : Cannot get frame from partial filter\n");
         return false;
    }
    return true;
}

/**
    \fn getInfo
    \brief Get FilterInfo
*/
FilterInfo  *partialFilter::getInfo(void)
{
    if (!sonFilterPreview)
        return previousFilter->getInfo();
    
    previewFilterInfo = *(previousFilter->getInfo());
    getRange(&(previewFilterInfo.markerA), &(previewFilterInfo.markerB));
    return &previewFilterInfo;
}

/**
 */
bool partialFilter::isInRange(uint64_t tme)
{
    tme += previousFilter->getAbsoluteStartTime();
    uint32_t tmeMs = tme / 1000LL;	// we need to comprare at ms resolution, otherwise if usec part not zero, would mess up things

    if(tmeMs<configuration.startBlack || tmeMs>=configuration.endBlack)
    {
        return false;
    }
    return true;
}

/**
    \fn getRange
    \fn Get the time a partialized filter should be active
*/
bool partialFilter::getRange(uint64_t *startTme, uint64_t *endTme)
{
    *startTme=1000LL*configuration.startBlack;
    *endTme=1000LL*configuration.endBlack;
    return true;
}

/**
 */
bool         partialFilter::goToTime(uint64_t usSeek)
{
    bool r=previousFilter->goToTime(usSeek);
    byPass=!isInRange(usSeek);
    if(!byPass)
        sonFilter->goToTime(usSeek);
    return r;
}

/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         partialFilter::getCoupledConf(CONFcouple **couples)
{
    *couples=NULL;
    CONFcouple *newParam=NULL;
    sonFilter->getCoupledConf(&newParam);
    if(newParam)
    {
        if(configuration.startBlack > configuration.endBlack)
        {
            uint32_t swap=configuration.startBlack;
            configuration.startBlack=configuration.endBlack;
            configuration.endBlack=swap;
        }
        if(configuration.endBlack > info.totalDuration)
            configuration.endBlack=info.totalDuration;

        *couples=new CONFcouple(newParam->getSize()+3);
        (*couples)->writeAsString("filterName",configuration.filterName.c_str());
        (*couples)->writeAsUint32("startBlack",configuration.startBlack);
        (*couples)->writeAsUint32("endBlack",configuration.endBlack);

        for(int i=0;i<newParam->getSize();i++)
        {
          char *key,*val;
          newParam->getInternalName (i,&key,&val);
          (*couples)->setInternalName(key,val);
        }
        delete newParam;
        newParam=NULL;
    }
    return true;
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
    uint32_t id = ADM_vf_getTagFromInternalName(configuration.filterName.c_str());
    int len = DESC_MAX_LENGTH;

    snprintf(description,len,"%s: ",ADM_vf_getDisplayNameFromTag(id));

    len -= strlen(description);
    if(len < 1)
        return description;

    const char *str = ADM_us2plain((uint64_t)(configuration.startBlack)*1000);
    len -= strlen(str)+4;
    if(len < 1)
        return description;

    strcat(description,str);
    strcat(description," -- ");

    str = ADM_us2plain((uint64_t)(configuration.endBlack)*1000);
    len -= strlen(str)+1;
    if(len < 1)
        return description;

    strcat(description,str);
    strcat(description,"\n");

    str = sonFilter->getConfiguration();
    len -= strlen(str);
    if(len < 1)
        return description;

    strcat(description,str);

    return description;
}
/**
 */
void partialFilter::reconfigureCallback(void *cookie)
{
  partialFilter *p=(partialFilter *)cookie;
  p->reconfigureSon();
}
/**
 */
void partialFilter::reconfigureSon(void)
{
  sonFilterPreview=true;
  sonFilter->configure();
  sonFilterPreview=false;
}

/**
    \fn configure
*/
bool partialFilter::configure( void)
{
    uint32_t mx = (uint32_t)(previousFilter->getInfo()->totalDuration/1000);
    uint32_t id = ADM_vf_getTagFromInternalName(configuration.filterName.c_str());
    char str[256];
    str[0] = '\0';
    snprintf(str,256,QT_TRANSLATE_NOOP("partial","Partialize \"%s\""),ADM_vf_getDisplayNameFromTag(id));
    str[255] = '\0';

    diaElemTimeStamp start(&configuration.startBlack, QT_TRANSLATE_NOOP("partial","_Start time:"), 0, mx);
    diaElemTimeStamp end(&configuration.endBlack, QT_TRANSLATE_NOOP("partial","_End time:"), 0, mx);
    diaElemButton son(QT_TRANSLATE_NOOP("partial", "Configure filter"), partialFilter::reconfigureCallback, this);

    diaElem *elems[3]={&start,&end,&son};
    return diaFactoryRun(str,3,elems);
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

bool partialFilter::trampolineFilter::getTimeRange(uint64_t *start, uint64_t *end)
{
    return ((partialFilter *)previousFilter)->getRange(start,end);
}

bool partialFilter::trampolineFilter::goToTime(uint64_t usSeek)
{
    partialFilter *p=(partialFilter *)previousFilter;
    return p->previousFilter->goToTime(usSeek);
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

  uint32_t start, end;
  uint64_t startPts, endPts;
  startPts = source->getInfo()->markerA;
  endPts = source->getInfo()->markerB;

  start = startPts/1000;
  end = endPts/1000;

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
  if(!p->configure())
  {
      delete p;
      p=NULL;
      return NULL;
  }
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
