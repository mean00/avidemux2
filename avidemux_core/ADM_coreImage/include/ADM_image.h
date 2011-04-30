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
#include "ADM_assert.h"
#include "ADM_rgb.h"
#include "ADM_colorspace.h"

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
        PLANAR_LAST=3
        
} ADM_PLANE;

typedef enum
{
    ADM_IMAGE_DEFAULT,
    ADM_IMAGE_REF,
    ADM_IMAGE_VDPAU
}ADM_IMAGE_TYPE;
#define YPLANE(x) ((x)->GetReadPtr(PLANAR_Y))
#define UPLANE(x) ((x)->GetReadPtr(PLANAR_U))
#define VPLANE(x) ((x)->GetReadPtr(PLANAR_V))
class ADMImageRef;
class ADMImageDefault;
/**
    \class ADMImage
    \brief Stores image

*/
class ADMImage
{
public: // half public/protected, only in  ADMImageRef case it is really public
        uint8_t         *_planes[3];     /// In case of linked data store y/u/v pointers
        uint32_t        _planeStride[3]; /// Same story
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
        // Quant info
        uint8_t         *quant;
        int             _qStride;
        int             _qSize;
        int             GetHeight(ADM_PLANE plane) {if(plane==PLANAR_Y) return _height; return _height/2;}
        bool            GetPitches(uint32_t *pitches) {pitches[0]=GetPitch(PLANAR_Y);
                                                       pitches[1]=GetPitch(PLANAR_U);
                                                       pitches[2]=GetPitch(PLANAR_V);}
        bool            GetWritePlanes(uint8_t **planes) {planes[0]=GetWritePtr(PLANAR_Y);
                                                        planes[1]=GetWritePtr(PLANAR_U);
                                                        planes[2]=GetWritePtr(PLANAR_V);}
        bool            GetReadPlanes(uint8_t **planes) {planes[0]=GetReadPtr(PLANAR_Y);
                                                         planes[1]=GetReadPtr(PLANAR_U);
                                                         planes[2]=GetReadPtr(PLANAR_V);}

virtual                 ~ADMImage();        
        

protected:
        ADMImage(uint32_t width, uint32_t height,ADM_IMAGE_TYPE type);
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
        bool    merge(ADMImage *src1,ADMImage *src2);
        bool    substract(ADMImage *src1,ADMImage *src2);
        bool    blacken(void);
        bool    copyTo(ADMImage *target, uint32_t x, uint32_t y);
        bool    copyToAlpha(ADMImage *target, uint32_t x, uint32_t y,uint32_t alpha);
        bool    copyLeftSideTo(ADMImage *dest);
        /* Some utilitarian functions */
        bool    saveAsBmp(const char *filename);
        bool    saveAsJpg(const char *filename);
        bool    printString(uint32_t x,uint32_t y, const char *strng);
static  bool    copyPlane(ADMImage *s, ADMImage *d, ADM_PLANE plane);
static  uint32_t lumaDiff(ADMImage *src1,ADMImage *src2,uint32_t noise);
};

/**
    \class ADMImageDefault
*/
class ADMImageDefault: public ADMImage
{
protected:
                    uint8_t         *data;
public:
                                    ADMImageDefault(uint32_t w, uint32_t h);
        virtual      ~ADMImageDefault();
        virtual      uint32_t        GetPitch(ADM_PLANE plane);
        virtual      uint8_t        *GetWritePtr(ADM_PLANE plane);
        virtual      uint8_t        *GetReadPtr(ADM_PLANE plane);
        virtual      bool           isWrittable(void);
};
/**
    \class ADMImageRef
    \brief That image is a shell for another image. You cannot write to it!
*/
class ADMImageRef: public ADMImage
{
public:

public:
                        ADMImageRef(uint32_t w, uint32_t h);
        virtual      ~ADMImageRef();
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

void drawString(ADMImage *dst, int x, int y, const char *s) ;

// Misc utilities
bool BitBlit(uint8_t *dst, uint32_t pitchDest,uint8_t *src,uint32_t pitchSrc,uint32_t width, uint32_t height);
bool BitBlitAlpha(uint8_t *dst, uint32_t pitchDst,uint8_t *src,uint32_t pitchSrc,uint32_t width, uint32_t height,uint32_t alpha);

#endif
