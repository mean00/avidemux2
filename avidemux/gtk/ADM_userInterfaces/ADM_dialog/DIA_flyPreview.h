#ifndef FLY_PREVIEW_H
#define FLY_PREVIEW_H
class flyPreview : public ADM_flyDialogGtk
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
	  ADM_flyDialogGtk(width, height, NULL, canvas, NULL, true, RESIZE_AUTO) 
        {
            
        };
	virtual ~flyPreview(void) {};
    bool setCurrentPts(uint64_t pts) {return 1;}
};

class flySeekablePreview : public ADM_flyDialogGtk
{
public:
    uint8_t processYuv(ADMImage *in,ADMImage *out) 
            {
                out->duplicate(in);
                return 1;
            }

	uint8_t download(void) {return 1;}
	uint8_t upload(void) {return 1;}
	uint8_t cleanup(void) {return 1;}
    bool setCurrentPts(uint64_t pts) {return 1;}
	flySeekablePreview(uint32_t width, uint32_t height, ADM_coreVideoFilter *videoStream, void *canvas, void *slider) : 
	  ADM_flyDialogGtk(width, height, videoStream, canvas, slider, 0, RESIZE_AUTO) 
            {};
	virtual ~flySeekablePreview(void) {};
};
#endif
