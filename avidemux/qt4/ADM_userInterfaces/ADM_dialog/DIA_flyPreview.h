#ifndef FLY_PREVIEW_H
#define FLY_PREVIEW_H
class flyPreview : public ADM_flyDialogQt4
{
public:
	uint8_t process(void) {return 1;}
	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}
	uint8_t setData(uint8_t *buffer) 
        {
#warning FIXME  NO DISPLAY   
            action->process();
            //_rgb->convert(buffer, _rgbBufferOut); 
            return 1;
        }

	flyPreview(uint32_t width, uint32_t height, void *canvas) : 
	  ADM_flyDialogQt4(width, height, NULL, canvas, NULL, true, RESIZE_NONE) 
            {
                
            };
	virtual ~flyPreview(void) 
            {

            };
};

typedef bool (*CookieFunc)(void *c,uint64_t pts);
/**
    \class flySeekablePreview
*/
class flySeekablePreview : public ADM_flyDialogQt4
{
protected:
    void *cookie;
    CookieFunc cookieFunc;
public:
	uint8_t processYuv(ADMImage *in,ADMImage *out) 
            {
                out->duplicate(in);
                return 1;
            }
	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}
    bool    setCurrentPts(uint64_t pts)
            {
                if(cookieFunc)
                    cookieFunc(cookie,pts);
                return true;
            }
	flySeekablePreview(uint32_t width, uint32_t height, ADM_coreVideoFilter *videoStream, void *canvas, void *slider) : 
	  ADM_flyDialogQt4(width, height, videoStream, canvas, slider, true, RESIZE_LAST) 
        {
                cookie=NULL;cookieFunc=NULL;
        };
	virtual ~flySeekablePreview(void) {};
    bool setCookieFunc(CookieFunc cookieFunc, void *cookie)
        {
                this->cookieFunc=cookieFunc;
                this->cookie=cookie;
                return true;
        }
};
#endif
