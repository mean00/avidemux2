//
// C++ Interface: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
//	This is the base time for image exchanged between codec/filters/...
//
//	We (optionnally) can carry extra informations
//		- aspect ratio
//		- frame type
//		- quantizer for each macroblock (16x16 pixels)
//      - PTS : Presentation time in us of the image
//	For the latter 3 infos are used
//		quant which leads to the int8 quant array
//		qstride = stride of array. Usually width+15)/16. 0 MEANS NOT USABLE
//		qsize = size of the array (needed to be able to copy it)
//
#ifndef ADM_IMAGE
#define ADM_IMAGE
#include "ADM_coreImage6_export.h"
#include "ADM_inttype.h"
#include "ADM_rgb.h"
#include "ADM_colorspace.h"
#include "ADM_assert.h"
#include "ADM_byteBuffer.h"

typedef enum
{
	ADM_ASPECT_4_3=1,
	ADM_ASPECT_16_9,
	ADM_ASPECT_1_1
} ADM_ASPECT;
// Avisynth compatibility layer

//#define vi.num_frames _info.nb_frames
//#define vi.IsYV12()   1
#define GetRowSize GetPitch

typedef enum
{
        PLANAR_Y=0,
        PLANAR_U=1,
        PLANAR_V=2,
        PLANAR_ALPHA=3,
        PLANAR_LAST=3 // Alpha is not a real channel

} ADM_PLANE;

typedef enum
{
    ADM_IMAGE_DEFAULT,
    ADM_IMAGE_REF
}ADM_IMAGE_TYPE;

typedef enum
{
        ADM_HW_NONE,
        ADM_HW_VDPAU,
        ADM_HW_LIBVA,
        ADM_HW_DXVA,
        ADM_HW_ANY=0xff
}ADM_HW_IMAGE;


typedef bool refFunction(void *instance,void *cookie);
typedef bool refDownloadFunction(ADMImage *image, void *refCodec, void *refHwImage);
/**
    \struct hwRefDescriptor
    \brief  Used to deal with hw accelerated stuff
*/
typedef struct
{
        void            *refCodec;  ///
        void            *refHwImage;  /// Ref to a hw image
        refFunction     *refMarkUsed;   ///
        refFunction     *refMarkUnused; ///
        refDownloadFunction     *refDownload;
}hwRefDescriptor;

#define YPLANE(x) ((x)->GetReadPtr(PLANAR_Y))
#define UPLANE(x) ((x)->GetReadPtr(PLANAR_U))
#define VPLANE(x) ((x)->GetReadPtr(PLANAR_V))
class ADMImageRef;
class ADMImageDefault;
/**
    \class ADMImage
    \brief Stores image

*/
class ADM_COREIMAGE6_EXPORT ADMImage
{
public: // half public/protected, only in  ADMImageRef case it is really public
        uint8_t         *_planes[3];     /// In case of linked data store y/u/v pointers
        int             _planeStride[3]; /// Same story
        uint8_t         *_alpha;         /// Null in most case, else alha channel
        int             _alphaStride;
public:
        uint32_t	    _width;		/// Width of image
        uint32_t	    _height;	/// Height of image
        uint32_t	    _Qp;		/// Average quantizer for this image, Default=2
        uint32_t	    flags;		/// Flags for this image (AVI_KEY_FRAME/AVI_B_FRAME)
        uint64_t        Pts;        /// Presentation time in us
        ADM_IMAGE_TYPE  _imageType;     /// Plain image or reference or vdpau wrapper
        ADM_colorspace  _colorspace;    /// Colorspace we are moving, default is YV12
        uint8_t         _noPicture;     /// No picture to display
        ADM_ASPECT	    _aspect;	/// Aspect ratio
        //
        ADM_HW_IMAGE    refType;    /// if not none, it means the field below is a ref to a hw image
        hwRefDescriptor refDescriptor;
        // Quant info
        uint8_t         *quant;
        int             _qStride;
        int             _qSize;


        int             GetHeight(ADM_PLANE plane) ;
        int             GetWidth(ADM_PLANE plane) ;
        bool            GetPitches(int *pitches);
        bool            GetWritePlanes(uint8_t **planes);
        bool            GetReadPlanes(uint8_t **planes);

virtual                 ~ADMImage();


protected:
                                    ADMImage(uint32_t width, uint32_t height,ADM_IMAGE_TYPE type);
                    bool            saveAsBmpInternal(const char *filename);
                    bool            saveAsJpgInternal(const char *filename);
public:
                     bool            hwIncRefCount(void);
                     bool            hwDecRefCount(void);
                     bool            hwDownloadFromRef(void);
public:
        virtual      uint32_t        GetPitch(ADM_PLANE plane)=0;
        virtual      uint8_t        *GetWritePtr(ADM_PLANE plane)=0;
        virtual      uint8_t        *GetReadPtr(ADM_PLANE plane)=0;
        virtual      bool           isWrittable(void)=0;
        virtual      ADMImageRef    *castToRef(void) {return NULL;};

        virtual      bool           duplicateMacro(ADMImage *src,bool swap);       /// copy an image to ourself, including info

                     uint8_t        getWidthHeight(uint32_t *w,uint32_t *h)
                                    {
                                          *w=_width;
                                          *h=_height;
                                          return 1;
                                    }
        bool    duplicate(ADMImage *src);	/// copy an image to ourself, including info
        bool    duplicateFull(ADMImage *src);	/// copy an image to ourself, including info
        bool    copyInfo(ADMImage *src);	/// copy all the flags, not the data themselves
        bool    copyQuantInfo(ADMImage *src);	/// copy quant table if any
        bool    isRef(void) { if(_imageType==ADM_IMAGE_REF) return true;return false;};
        bool    blacken(void);
        bool    copyTo(ADMImage *target, uint32_t x, uint32_t y);
        bool    copyToAlpha(ADMImage *target, uint32_t x, uint32_t y,uint32_t alpha);
        bool    copyWithAlphaChannel(ADMImage *target, uint32_t x, uint32_t y);
        bool    copyLeftSideTo(ADMImage *dest);
        /* Some utilitarian functions */
        bool    saveAsBmp(const char *filename);

        bool    saveAsJpg(const char *filename);
        bool    printString(uint32_t x,uint32_t y, const char *strng);
static  bool    copyPlane(ADMImage *s, ADMImage *d, ADM_PLANE plane);

        bool    convertFromYUV444(uint8_t *from);
        bool    convertFromNV12(uint8_t *yData, uint8_t *uvData, int strideY, int strideUV);
        bool    convertToNV12(uint8_t *yData, uint8_t *uvData, int strideY, int strideUV);
        bool    interleaveUVtoNV12(uint8_t *target, int targetStride);
};

/**
    \class ADMImageDefault
*/
class ADM_COREIMAGE6_EXPORT ADMImageDefault: public ADMImage
{
protected:
                    ADM_byteBuffer  data;
                    ADM_byteBuffer  alphaChannel;
public:
                                    ADMImageDefault(uint32_t w, uint32_t h);
        virtual                     ~ADMImageDefault();
        virtual      uint32_t        GetPitch(ADM_PLANE plane);
        virtual      uint8_t        *GetWritePtr(ADM_PLANE plane);
        virtual      uint8_t        *GetReadPtr(ADM_PLANE plane);
        virtual      bool           isWrittable(void);
                     bool           addAlphaChannel();
};
/**
    \class ADMImageRef
    \brief That image is a shell for another image. You cannot write to it!
*/
class ADM_COREIMAGE6_EXPORT ADMImageRef: public ADMImage
{
public:

public:
                                    ADMImageRef(uint32_t w, uint32_t h);
        virtual                     ~ADMImageRef();
        virtual      uint32_t        GetPitch(ADM_PLANE plane);
        virtual      uint8_t        *GetWritePtr(ADM_PLANE plane);
        virtual      uint8_t        *GetReadPtr(ADM_PLANE plane);
        virtual      bool           isWrittable(void);
        virtual      ADMImageRef    *castToRef(void) {return this;};
};
/**
    \class ADMImageRefWrittable
    \brief That image is a shell for another image. You can write to it!
*/
class ADMImageRefWrittable: public ADMImageRef
{
public:
                 ADMImageRefWrittable(uint32_t w, uint32_t h) : ADMImageRef(w,h) {};
    virtual      bool           isWrittable(void) {return true;}
    virtual      uint8_t        *GetWritePtr(ADM_PLANE plane) {return GetReadPtr(plane);}
};

#define drawString(img,x,y,text) img->printString(x,y,text)
//void drawString(ADMImage *dst, int x, int y, const char *s) ;

// Misc utilities
ADM_COREIMAGE6_EXPORT bool BitBlit(uint8_t *dst, uint32_t pitchDest,uint8_t *src,uint32_t pitchSrc,uint32_t width, uint32_t height);
bool BitBlitAlpha(uint8_t *dst, uint32_t pitchDst,uint8_t *src,uint32_t pitchSrc,uint32_t width, uint32_t height,uint32_t alpha);
ADM_COREIMAGE6_EXPORT void ADMImage_stat(void);

#endif
