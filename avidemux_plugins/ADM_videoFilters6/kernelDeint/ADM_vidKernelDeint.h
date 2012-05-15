
#ifndef ADM_KERNELDEINT
#define ADM_KERNELDEINT
#include "kdeint.h"
/**
    \class kernelDeint
*/
class kernelDeint : public  ADM_coreVideoFilterCached
{
protected:
        kdeint       param;
        bool         debug;
        
public:
                    kernelDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                    ~kernelDeint();

        virtual const char   *getConfiguration(void);                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
	 //  virtual FilterInfo  *getInfo(void);                             /// Return picture parameters after this filter
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;           /// Start graphical user interface
};


#endif
