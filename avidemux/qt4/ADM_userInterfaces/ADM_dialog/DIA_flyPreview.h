#ifndef FLY_PREVIEW_H
#define FLY_PREVIEW_H
class flyPreview : public ADM_flyDialogQt4
{
public:
	uint8_t process(void) {return 1;}
	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}
	flyPreview(uint32_t width, uint32_t height, ADM_QCanvas *canvas) : 
	  ADM_flyDialogQt4(width, height, NULL, canvas, NULL, true, RESIZE_AUTO) 
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
        flySeekablePreview(uint32_t width, uint32_t height, ADM_coreVideoFilter *videoStream, ADM_QCanvas *canvas, QSlider *slider) : 
	  ADM_flyDialogQt4(width, height, videoStream, canvas, slider, true, RESIZE_AUTO) 
        {
                cookie=NULL;cookieFunc=NULL;
        };
        virtual ~flySeekablePreview(void) {};
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
            if(cookieFunc)              // Used as a callback to update timestamp
                cookieFunc(cookie,pts);
            return true;
        }
	
        bool setCookieFunc(CookieFunc cookieFunc, void *cookie)
        {
            this->cookieFunc=cookieFunc;
            this->cookie=cookie;
            return true;
        }
};
#endif
