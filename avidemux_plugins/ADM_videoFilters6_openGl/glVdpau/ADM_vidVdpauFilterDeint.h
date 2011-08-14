/**
    \brief VDPAU filters Deinterlacer
    \author mean (C) 2010
  
    This version uses openGL to convert the output surface to YV12


*/

//
#define ADM_INVALID_FRAME_NUM 0x80000000
#define ADM_NB_SURFACES 5


enum
{
    ADM_KEEP_TOP=0,
    ADM_KEEP_BOTTOM=1,
    ADM_KEEP_BOTH=2
};
/**
    \class VDPSlot
*/
class VDPSlot
{
public:
                              VDPSlot() ;
                             ~VDPSlot();
            VdpVideoSurface   surface;
            bool              isExternal;
            uint64_t          pts;
            uint32_t          frameNumber;
            ADMImage          *image;
};
/**
    \class glRGB
*/
class glRGB : public  ADM_coreVideoFilterQtGl
{
protected:

protected:
                            bool render(ADMImage *image,ADM_PLANE plane,QGLFramebufferObject *fbo);
                    
public:
                             glRGB(ADM_coreVideoFilter *previous,CONFcouple *conf);
                            ~glRGB();

        virtual const char   *getConfiguration(void) {return "none";};                   /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);    /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) {return true;}             /// Start graphical user interface

        bool                 surfaceToImage(VdpOutputSurface surf,ADMImage *image);
};

/**
    \class vdpauVideoFilterDeint
*/
class vdpauVideoFilterDeint : public  ADM_coreVideoFilter
{
protected:
                    VDPSlot              xslots[3];
                    bool                 eof;
                    bool                 secondField;
                    uint64_t             nextPts;
                    ADMColorScalerSimple *scaler;
                    bool                 passThrough;
                    bool                 setupVdpau(void);
                    bool                 cleanupVdpau(void);
                    bool                 updateConf(void);
                    uint8_t             *tempBuffer;
                    vdpauFilterDeint     configuration;
                    VdpOutputSurface     outputSurface;
                    std::list <VdpVideoSurface> freeSurface;
                    VdpVideoSurface      surfacePool[ADM_NB_SURFACES];
                    VdpVideoMixer        mixer;
                    glRGB                *rgb;
protected:
                    bool                initGl(void);
                    bool                initOnceGl(void);
                    bool                deInitGl(void);
protected:
                    bool                 rotateSlots(void);
                    bool                 clearSlots(void);
                    bool                 uploadImage(ADMImage *next,const VdpVideoSurface surface) ;
                    bool                 fillSlot(int slot,ADMImage *image);
                    bool                 getResult(ADMImage *image);
                    bool                 sendField(bool topField);

public:
        virtual bool         goToTime(uint64_t usSeek); 
                             vdpauVideoFilterDeint(ADM_coreVideoFilter *previous,CONFcouple *conf);
                             ~vdpauVideoFilterDeint();

        virtual const char   *getConfiguration(void);                 /// Return  current configuration as a human readable string
        virtual bool         getNextFrame(uint32_t *fn,ADMImage *image);           /// Return the next image
        virtual bool         getCoupledConf(CONFcouple **couples) ;   /// Return the current filter configuration
        virtual bool         configure(void) ;                        /// Start graphical user interface
};

//EOF

// EOF
