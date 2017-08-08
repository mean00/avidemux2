#ifndef FLY_PREVIEW_H
#define FLY_PREVIEW_H
class flyPreview : public ADM_flyDialogYuv
{
public:
	bool    process(void) {return 1;}
	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}
                flyPreview(QDialog *parent,uint32_t width, uint32_t height, ADM_QCanvas *canvas) : 
                    ADM_flyDialogYuv(parent,width, height, NULL, canvas, NULL,  RESIZE_AUTO) 
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
class flySeekablePreview : public ADM_flyDialogYuv
{
protected:
    void *cookie;
    CookieFunc cookieFunc;
public:
        flySeekablePreview(QDialog *parent,uint32_t width, uint32_t height, ADM_coreVideoFilter *videoStream, ADM_QCanvas *canvas, ADM_QSlider *slider) :
	  ADM_flyDialogYuv(parent,width, height, videoStream, canvas, slider,  RESIZE_AUTO) 
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
