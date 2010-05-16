/**
    \brief VDPAU filters
    \author mean (C) 2010
    This is slow as we copy back and forth data to/from the video cards
    Inspired a lot from mplayer vo_vdpau

*/

#include "ADM_default.h"
#include "ADM_coreVideoFilterInternal.h"
#include "ADM_videoFilterCache.h"
#include "DIA_factory.h"
#include "vdpauFilter.h"
#include "vdpauFilter_desc.cpp"
//
/**
    \class vdpauVideoFilter
*/
class vdpauVideoFilter : public  ADM_coreVideoFilter
{
protected:
                    ADMImage    *original;
                    vdpauFilter configuration;
public:
                    vdpauVideoFilter(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~vdpauVideoFilter();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;                        /// Start graphical user interface
};

// Add the hook to make it valid plugin
DECLARE_VIDEO_FILTER(   vdpauVideoFilter,   // Class
                        1,0,0,              // Version
                        ADM_UI_GTK+ADM_UI_QT4,         // We need a display for VDPAU; so no cli...
                        VF_INTERLACING,            // Category
                        "vdpau",            // internal name (must be uniq!)
                        "vdpau",            // Display name
                        "vdpau, vdpau filters, SLOW." // Description
                    );

//
static void filter_plane(int mode, uint8_t *dst, int dst_stride, const uint8_t *prev0, const uint8_t *cur0, const uint8_t *next0, int refs, int w, int h, int parity, int tff, int mmx);


/**
    \fn constructor
*/
vdpauVideoFilter::vdpauVideoFilter(ADM_coreVideoFilter *in, CONFcouple *setup): ADM_coreVideoFilter(in,setup)
{
    original=new ADMImageDefault(in->getInfo()->width,in->getInfo()->height);
    if(!setup || !ADM_paramLoad(setup,vdpauFilter_param,&configuration))
    {
        // Default value
        configuration.mode=0;
        configuration.order=1;
    }
    vidCache = new VideoCache (10, in);
    myName="vdpau";
}
/**
    \fn destructor
*/
vdpauVideoFilter::~vdpauVideoFilter()
{
        delete  original;
        original=NULL;
       
        delete vidCache;
        vidCache = NULL;
}
/**
    \fn updateInfo
*/
bool vdpauVideoFilter::configure( void) 
{
    
     diaMenuEntry tMode[]={
                             {0,      QT_TR_NOOP("Temporal & spatial check"),NULL},
                             {1,   QT_TR_NOOP("Bob, temporal & spatial check"),NULL},
                             {2,      QT_TR_NOOP("Skip spatial temporal check"),NULL},
                             {3,  QT_TR_NOOP("Bob, skip spatial temporal check"),NULL}
          };
     diaMenuEntry tOrder[]={
                             {0,      QT_TR_NOOP("Bottom field first"),NULL},
                             {1,   QT_TR_NOOP("Top field first"),NULL}
          };
  
     diaElemMenu mMode(&(configuration.mode),   QT_TR_NOOP("_Mode:"), 4,tMode);
     diaElemMenu morder(&(configuration.order),   QT_TR_NOOP("_Order:"), 2,tOrder);
     
     diaElem *elems[]={&mMode,&morder};
     
     if(diaFactoryRun(QT_TR_NOOP("vdpau"),sizeof(elems)/sizeof(diaElem *),elems))
     {
        return 1;
     }
     return 0;
}
/**
    \fn getCoupledConf
    \brief Return our current configuration as couple name=value
*/
bool         vdpauVideoFilter::getCoupledConf(CONFcouple **couples)
{
    return ADM_paramSave(couples, vdpauFilter_param,&configuration);
}
/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
const char *vdpauVideoFilter::getConfiguration(void)
{
    static char conf[80];
    conf[0]=0;
    snprintf(conf,80,"vdpau : mode=%d, order=%d\n",
                (int)configuration.mode, (int)configuration.order);
    return conf;
}

/**
    \fn getConfiguration
    \brief Return current setting as a string
*/
bool vdpauVideoFilter::getNextFrame(uint32_t *fn,ADMImage *image)
{

        
      return false;
}
//****************
// EOF
